#pragma once
#ifndef BL_ASSET_H
#define BL_ASSET_H

#include <blink/base.h>

typedef uint32_t BLAssetId;     ///< An identifier that uniquely identifies an asset of a type.
typedef uint32_t BLAssetTypeId; ///< An identifier that uniquely identifies an asset type.
typedef uint32_t BLAssetFence;  ///< An opaque handle for a synchronization fence.

/// Initializes the library. This must be called before any other library
/// functions can be used.
void bl_asset_init();

/// Shuts down the library. You must unload all assets and wait for them to
/// be unloaded before calling this. All allocated memory and system resources
/// will be freed.
void bl_asset_shutdown();

/// Function prototype for loading an asset into memory.
///
/// @param id         the unique id for the asset
/// @param version    the asset's format version
/// @param data       the data buffer containing the asset
/// @param data_size  the size of `data` in bytes
///
/// @return   a pointer to the loaded asset
typedef void* (*BLAssetLoadFunc)(BLAssetId id, uint32_t version, char* data, uint32_t data_size);

/// Function prototype for unloading an asset from memory.
///
/// @param id         the unique id for the asset
/// @param asset      the asset that was returned from a `BLAssetLoadFunc` call
typedef void (*BLAssetUnloadFunc)(BLAssetId id, void* asset);

/// Describes how a type gets loaded through the asset system.
struct BLAssetTypeHandler {
  BLAssetLoadFunc load;     ///< the function used to load an asset
  BLAssetUnloadFunc unload; ///< the function used to unload an asset
};

/// Registers a new asset type handler that knows how to load assets of that type.
///
/// @param type_id    the identifier for the type being described
/// @param handler    describes how to load/unload the type
void bl_asset_type_register(BLAssetTypeId type_id, const BLAssetTypeHandler* handler);

/// Unregisters an asset type handler previously registered with `bl_asset_type_register`.
///
/// @param type_id    the identifier for the type being described
void bl_asset_type_unregister(BLAssetTypeId type_id);

/// Queues an async request to load an asset into memory. The background worker
/// thread will eventually process the request and bring the asset into memory.
///
/// @param type_id    the type of asset to load
/// @param id         the id of the asset to load
void bl_asset_load(BLAssetTypeId type_id, BLAssetId id);

/// Queues an async request to unload an asset from memory. The background
/// worker will eventually process the request and take the asset out of memory.
///
/// @param type_id    the type of asset to load
/// @param id         the id of the asset to load
void bl_asset_unload(BLAssetTypeId type_id, BLAssetId id);

/// Tests if the given resource is loaded into memory and can be retrieved.
///
/// @param type_id    the type of asset to test
/// @param id         the id of the asset to test
///
/// @return true if the asset can be retrieved; false otherwise
bool bl_asset_can_get(BLAssetTypeId type_id, BLAssetId id);

/// Retrieves a pointer to the asset. It must have already been loaded into memory.
///
/// @param type_id    the type of asset to retrieve
/// @param id         the id of the asset to retrieve
///
/// @return a pointer to the asset if it's in memory; nullptr otherwise
void* bl_asset_get(BLAssetTypeId type_id, BLAssetId id);

/// Adds a fence into the command stream that can be queried on later. Any
/// assets loaded prior to the fence can be safely considered in memory
/// once the fence has been reached.
///
/// @return a handle to the fence that was inserted into the request queue
BLAssetFence bl_asset_add_fence();

/// Tests if the given fence created by `bl_asset_add_fence` has been reached.
///
/// @return true if the fence has been reached; false otherwise
bool bl_asset_fence_reached(BLAssetFence fence);

// Processes requests that have completed on the background thread, finishing up their work.
// This must be called regularly by the main thread.
void bl_asset_update();


#endif
