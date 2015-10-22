#include <cassert>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <stdlib.h>
#include <thread>
#include <unordered_map>
#include "asset.h"

//
// constants
//

#define BL_ASSET_COMPILED_FILE_VERSION  0
#define BL_ASSET_COMPILED_FILE_ENDIAN   0x01020304


//
// types
//

enum OpType {
  OP_TYPE_LOAD,
  OP_TYPE_UNLOAD,
  OP_TYPE_FENCE,
};

struct AssetOpIn {
  OpType type;
  union {
    struct {
      BLAssetTypeId   type_id;
      BLAssetId       id;
      BLAssetLoadFunc load_func;
    } load;

    struct {
      BLAssetTypeId     type_id;
      BLAssetId         id;
      BLAssetUnloadFunc unload_func;
      void*             asset;
    } unload;

    struct {
      BLAssetFence fence;
    } fence;
  };
};

struct AssetOpOut {
  OpType        type;
  union {
    struct {
      BLAssetTypeId type_id;
      BLAssetId     id;
      void*         asset;
    } load;

    struct {
      BLAssetTypeId type_id;
      BLAssetId     id;
    } unload;

    struct {
      BLAssetFence fence;
    } fence;
  };
};

struct AssetLocator {
  BLAssetTypeId type_id;
  BLAssetId     id;

  bool operator==(const AssetLocator & other) const {
    return type_id == other.type_id && id == other.id;
  }
};

struct AssetLocatorHash {
  size_t operator()(const AssetLocator & loc) const {
    size_t h0 = std::hash<BLAssetTypeId>()(loc.type_id);
    size_t h1 = std::hash<BLAssetId>()(loc.id);
    return h0 ^ (h1 << 1);
  }
};


struct CompiledAssetFileHeader {
  uint32_t magic;             // [big endian] BLAR
  uint32_t version;           // [big endian] file format version
  uint32_t asset_type_id;     // the asset's type id
  uint32_t asset_id;          // the asset's id
  uint32_t asset_version;     // the asset's binary format version
  uint32_t asset_size_bytes;  // the asset's size in bytes
};


//
// variables
//

// map of handlers for asset types
static std::unordered_map<BLAssetTypeId, BLAssetTypeHandler> s_type_handlers;

// map of assets that are currently loaded into memory
static std::unordered_map<AssetLocator, void*, AssetLocatorHash> s_assets;

// the background worker thread
static char         s_thread_memory[sizeof(std::thread)];
static std::thread* s_thread;
static bool         s_shutdown;

// the input stream of ops for the worker thread
static std::deque<AssetOpIn>    s_ops_in;
static std::mutex               s_ops_in_mutex;
static std::condition_variable  s_ops_in_cond_var;

// the output stream of ops for the worker thread
static std::deque<AssetOpOut>   s_ops_out;
static std::mutex               s_ops_out_mutex;

static BLAssetFence             s_fence_issued;   // the most recently issued fence
static BLAssetFence             s_fence_reached;  // the most recently reached fence


//
// worker thread
//

static void process_op_load(BLAssetTypeId type_id, BLAssetId id, BLAssetLoadFunc load_func) {
  // open the file
  char filename[64];
  snprintf(filename, sizeof(filename), "db/%s/%08x/%08x", "osx", type_id, id);
  FILE* fh = fopen(filename, "rb");
  if (!fh) {
    // io error
    bl_log_error("failed to open file: path='%s'", filename);
    return;
  }

  // read the file header
  CompiledAssetFileHeader header;
  if (1 != fread(&header, sizeof(CompiledAssetFileHeader), 1, fh)) {
    // io error
    bl_log_error("failed to read file header: path='%s'", filename);
    fclose(fh);
    return;
  }
  if (header.magic != 'RALB') {
    // corrupt
    bl_log_error("file header magic mismatch: path='%s', got=%08x, expected=%08x", filename, header.magic, 'BLAR');
    fclose(fh);
    return;
  }
  if (header.version != BL_ASSET_COMPILED_FILE_VERSION) {
    // corrupt
    bl_log_error("file header version mismatch: path='%s'", filename);
    fclose(fh);
    return;
  }
  if (header.asset_type_id != type_id) {
    // corrupt
    bl_log_error("file header type id mismatch: path='%s'", filename);
    fclose(fh);
    return;
  }
  if (header.asset_id != id) {
    bl_log_error("file header asset id mismatch: path='%s'", filename);
    // corrupt
    fclose(fh);
    return;
  }

  // read in the asset
  char* asset_data = (char*)bl_alloc(header.asset_size_bytes, 4);
  if (!asset_data) {
    // memory exhaustion
    bl_log_error("memory exhaustion: path='%s'", filename);
    fclose(fh);
    return;
  }
  if (1 != fread(asset_data, header.asset_size_bytes, 1, fh)) {
    // io error
    bl_log_error("io error reading asset data: path='%s'", filename);
    bl_free(asset_data);
    fclose(fh);
    return;
  }

  // done with io
  fclose(fh);

  // load the asset
  void* asset = load_func(id, header.asset_version, asset_data, header.asset_size_bytes);

  // queue the result to be processed by the main thread
  AssetOpOut out;
  out.type          = OP_TYPE_LOAD;
  out.load.type_id  = type_id;
  out.load.id       = id;
  out.load.asset    = asset;
  {
    std::lock_guard<std::mutex> lock(s_ops_out_mutex);
    s_ops_out.push_back(out);
  }
}

static void process_op_unload(BLAssetTypeId type_id, BLAssetId id, BLAssetUnloadFunc unload_func, void* asset) {
  unload_func(id, asset);
  bl_free(asset);

  AssetOpOut out;
  out.type            = OP_TYPE_UNLOAD;
  out.unload.type_id  = type_id;
  out.unload.id       = id;
  {
    std::lock_guard<std::mutex> lock(s_ops_out_mutex);
    s_ops_out.push_back(out);
  }
}

static void process_op_fence(BLAssetFence fence) {
  AssetOpOut out;
  out.type        = OP_TYPE_FENCE;
  out.fence.fence = fence;
  {
    std::lock_guard<std::mutex> lock(s_ops_out_mutex);
    s_ops_out.push_back(out);
  }
}

// entry point for the worker thread
static void worker_thread() {
  bool shutdown = false;
  for (;;) {
    AssetOpIn op;

    // wait for an op to become available or a shutdown request
    {
      std::unique_lock<std::mutex> lock(s_ops_in_mutex);
      while (s_ops_in.empty() && !s_shutdown) {
        s_ops_in_cond_var.wait(lock);
      }
      if (s_ops_in.empty()) {
        shutdown = true;
      }
      else {
        op = s_ops_in.front();
        s_ops_in.pop_front();
      }
      lock.unlock();
    }

    // check for shutdown
    if (shutdown) {
      break;
    }

    // process op
    switch (op.type) {
    case OP_TYPE_LOAD:
      process_op_load(op.load.type_id, op.load.id, op.load.load_func);
      break;

    case OP_TYPE_UNLOAD:
      process_op_unload(op.unload.type_id, op.unload.id, op.unload.unload_func, op.unload.asset);
      break;

    case OP_TYPE_FENCE:
      process_op_fence(op.fence.fence);
      break;

    default:
      assert(false);
    }
  }
}


//
// local methods
//

static void queue_op(const AssetOpIn & op) {
  {
    std::lock_guard<std::mutex> lock(s_ops_in_mutex);
    s_ops_in.push_back(op);
  }
  s_ops_in_cond_var.notify_one();
}


//
// api
//

void bl_asset_lib_initialize() {
  // init the fence
  s_fence_issued = 0;
  s_fence_reached = 0;

  // start the worker thread
  s_shutdown = false;
  s_thread = new (s_thread_memory) std::thread(&worker_thread);
}

void bl_asset_lib_finalize() {
  assert(s_type_handlers.empty());

  // ask the worker thread to shutdown and wait
  s_shutdown = true;
  s_ops_in_cond_var.notify_one();
  s_thread->join();
  s_thread->~thread();
  s_thread = nullptr;
}

void bl_asset_type_register(BLAssetTypeId type_id, const BLAssetTypeHandler * handler) {
  assert(s_type_handlers.find(type_id) == s_type_handlers.end());
  s_type_handlers[type_id] = *handler;
}

void bl_asset_type_unregister(BLAssetTypeId type_id) {
  assert(s_type_handlers.find(type_id) != s_type_handlers.end());
  s_type_handlers.erase(type_id);
}

void bl_asset_load(BLAssetTypeId type_id, BLAssetId id) {
  auto iter = s_type_handlers.find(type_id);
  assert(iter != s_type_handlers.end());

  AssetOpIn op;
  op.type           = OP_TYPE_LOAD;
  op.load.type_id   = type_id;
  op.load.id        = id;
  op.load.load_func = iter->second.load;
  queue_op(op);
}

void bl_asset_unload(BLAssetTypeId type_id, BLAssetId id) {
  auto type_iter = s_type_handlers.find(type_id);
  assert(type_iter != s_type_handlers.end());

  // lookup the asset
  auto asset_iter = s_assets.find({type_id, id});
  assert(asset_iter != s_assets.end());

  AssetOpIn op;
  op.type               = OP_TYPE_UNLOAD;
  op.unload.type_id     = type_id;
  op.unload.id          = id;
  op.unload.unload_func = type_iter->second.unload;
  op.unload.asset       = asset_iter->second;
  queue_op(op);
}

bool bl_asset_can_get(BLAssetTypeId type_id, BLAssetId id) {
  AssetLocator loc;
  loc.type_id = type_id;
  loc.id = id;
  return s_assets.find(loc) != s_assets.end();
}

void* bl_asset_get(BLAssetTypeId type_id, BLAssetId id) {
  AssetLocator loc;
  loc.type_id = type_id;
  loc.id = id;
  auto asset_iter = s_assets.find(loc);
  if (asset_iter == s_assets.end()) {
    return nullptr;
  }
  else {
    return asset_iter->second;
  }
}

BLAssetFence bl_asset_add_fence() {
  ++s_fence_issued;

  AssetOpIn op;
  op.type = OP_TYPE_FENCE;
  op.fence.fence = s_fence_issued;
  queue_op(op);

  return s_fence_issued;
}

bool bl_asset_fence_reached(BLAssetFence fence) {
  if (fence <= s_fence_reached) {
    return true;
  }
  return false;
}

void bl_asset_update() {
  std::lock_guard<std::mutex> lock(s_ops_out_mutex);
  while (!s_ops_out.empty()) {
    const AssetOpOut & op = s_ops_out.front();
    switch (op.type) {
      case OP_TYPE_LOAD:
        {
          AssetLocator loc = { op.load.type_id, op.load.id };
          s_assets[loc] = op.load.asset;
        }
        break;

      case OP_TYPE_UNLOAD:
        s_assets.erase({op.unload.type_id, op.unload.id});
        break;

      case OP_TYPE_FENCE:
        s_fence_reached = op.fence.fence;
        break;

      default:
        assert(false);
    }

    s_ops_out.pop_front();
  }
}
