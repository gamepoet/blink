require 'net/http'

module NeedMongo
  module ClassMethods
    def db
      if @db.nil?
        @db = Mongo::Connection.new('localhost').db('blink')
      end
      return @db
    end

    def fs
      if @gridfs.nil?
        @gridfs = Mongo::GridFileSystem.new(db)
      end
      return @gridfs
    end

    def grid
      if @grid.nil?
        @grid = Mongo::Grid.new(db)
      end
      return @grid
    end
  end

  def self.included(base)
    base.extend(ClassMethods)
  end
end

module NeedFaye
  module ClassMethods
    def publish(channel, data)
      msg = { :channel => channel, :data => data }
      uri = URI.parse('http://localhost:5000/faye')
      Net::HTTP.post_form(uri, :message => msg.to_json)
    end

#    def pubsub
#      if @pubsib.nil?
#        Faye.ensure_reactor_running!
#        @pubsub = Faye::Client.new('http://localhost:5000/faye')
#      end
#      return @pubsub
#    end
  end

  def self.included(base)
    base.extend(ClassMethods)
  end
end

class TextureSourceProcessorJob
  include NeedFaye
  include NeedMongo
  @queue = :textures_source_queue

  def self.perform(id, version)
    puts "[TextureSourceProcessor] id=#{id}, version=#{version}"
    # find the asset that needs to be built
    record = db.assets[:texture].find_one({
      :_id            => id,
      :version        => { :$gte => version },
      'status.source' => false,
    })
    # bail if the record could not be found (likely already built)
    if not record
      puts "Could not find texture: id=#{id}, version=#{version}"
      return
    end

    # get the specific source file associated with the version retrieved
    src_file = grid.get(record.file_ids.source)
    img = TextureCompiler::Image.load(src_file)
    src_file.close
    # TODO handle failure

    img_height = img.height
    img_width  = img.width
    #img.dispose
    # TODO dispose the image data to free up memory

    query = {
      :_id      => id,
      :version  => record.version,  # ensure the record hasn't changed while this job was working
    }
    update = {
      :$set => {
        :updated_at               => Time.now,
        'status.source'           => true,
        'metadata.source.height'  => img_height,
        'metadata.source.width'   => img_width,
      },
    }
    if not record.metadata.include?(:target)
      update[:$set].merge!({
        'metadata.target.default.format'    => 'dxt5',
        'metadata.target.default.height'    => img_height,
        'metadata.target.default.width'     => img_width,
        'metadata.target.default.levels'    => 1,
        'metadata.target.default.semantic'  => semantic_from_filename(record.filename)
      })
    end

    result = db.assets[:texture].find_and_modify(
      :query => query,
      :update => update,
      :new => true
    )
    #db.assets[:texture].update(query, update)
    # TODO handle failure

    publish("/assets/texture/#{id}", result.to_json)

    # queue up builds for the other platforms
    Resque.enqueue(TextureCompilerJob, id, :osx_x64, record.version)
  end

  def self.semantic_from_filename(filename)
    if filename =~ /normal$/i or filename =~ /_n$/i
      return 'normalmap'
    elsif filename =~ /specular$/i or filename =~ /_s$/i
      return 'specmap'
    else
      return 'diffuse'
    end
  end
end

class TextureCompilerJob
  include NeedFaye
  include NeedMongo
  @queue = :textures_platform_queue

  def self.perform(id, platform, version)
    puts "[TextureCompilerJob] id=#{id} platform=#{platform} version=#{version}"
    # find the asset that needs to be built
    record = db.assets['texture'].find_one({
      :_id                  => id,
      :version              => { :$gte => version },
      'status.source'       => true,
      :$or => [
        { "status.#{platform}"  => false },
        { "status.#{platform}"  => nil },
      ],
    })
    # bail if the record could not be found (likely already built)
    if not record
      puts "Could not find texture: id=#{id}, platform=#{platform}, version=#{version}"
      return
    end

    puts record

    # get the specific source file associated with the version retrieved
    src_file = grid.get(record.file_ids.source)
    img = TextureCompiler::Image.load(src_file)
    src_file.close
    # TODO handle failure

    dst_semantic  = get_metadata(record, platform, :semantic)
    dst_height    = get_metadata(record, platform, :height)
    dst_width     = get_metadata(record, platform, :width)

    puts "#{platform} #{dst_semantic} #{dst_height} #{dst_width}"

    # TODO compress to dxt (based the semantic)
    img_compressed = img.to_dxt

    # store the compressed surface in mongo
    dst_file_id = nil
    fs.open("/texture/#{id}/osx_x64", 'w') do |f|
      dst_file_id = f.files_id
      f.write(img_compressed.surface)
    end

    query = {
      :_id                        => id,
      :version                    => record.version,  # ensure the record hasn't changed while this job was working
    }
    update = {
      :$set => {
        :updated_at               => Time.now,
        "status.#{platform}"      => true,
        "file_ids.#{platform}"    => dst_file_id,
      },
    }
    result = db.assets[:texture].find_and_modify(
      :query => query,
      :update => update,
      :new => true
    )
    puts "committed: #{result.to_json}"
    puts 'job publish'
    publish("/assets/texture/#{id}", result.to_json)
    puts 'publish ok'
  end

  def self.get_metadata(record, platform, key)
    puts "get_metadata: #{key}"
    key_str = key.to_s

    # try the platform-specific version first
    target = record.metadata.target
    if target.include?(platform)
      platform_hash = target[platform]
      if platform_hash.include?(key_str)
        return platform_hash[key_str]
      end
    end

    # use the default
    return target['default'][key_str]
  end
end
