require 'bundler/setup'
Bundler.require(:default)
$:.push(File.dirname(__FILE__) + '/lib')
require 'digest/sha1'
require 'blink/ext'
require 'workers/texture_compiler'

STDOUT.sync = true

#
# db setup
#

$db = Mongo::Connection.new('localhost').db('blink')
$faye = Faye::Client.new('http://localhost:5000/faye')

ASSET_TYPES = %w(
  texture
).map { |x| x.freeze }.freeze

class Hash
  def flatten_with_path(prefix = nil)
    res = {}

    self.each do |key, val|
      if prefix
        key = "#{prefix}.#{key}"
      end

      if val.is_a? Hash
        res.merge!(val.flatten_with_path(key))
      else
        res[key] = val
      end
    end

    return res
  end
end

class App < Sinatra::Base
  Bundler.require(environment)
  register Sinatra::AssetPack
  register Sinatra::Ember

  set :root, File.dirname(__FILE__)
  set :haml, :format => :html5

  configure do |c|
    mime_type :woff, 'application/font-woff'
  end
  configure(:development) do |c|
    require 'pp'
    register Sinatra::Reloader
    c.also_reload '**/*.rb'
  end

  assets do
    js :app, [
      '/js/vendor/jquery-1.7.2.js',
      '/js/vendor/jquery-ui-1.8.20.custom.min.js',
      '/js/vendor/jquery.fileupload.js',
      '/js/ember-config.js',
      '/js/vendor/ember-0.9.8.1.js',
      '/js/vendor/underscore-1.3.3.js',
      '/js/vendor/faye-browser-0.8.2.js',
      '/js/app.js',
      '/js/templates.js',
    ]

    css :app, [
      '/css/vendor/yui-cssreset-3.4.1.css',
      '/css/screen.css'
    ]

    js_compression :uglify
  end

  ember do
    templates '/js/templates.js', [
      '/app/templates/**/*.hbs'
    ], :relative_to => 'app/templates'

    template_name_style :path
  end

  #
  # routes
  #

  get '/' do
    haml :index
  end

  get '/session' do
    session = $db.sessions.find_one()
    if not session
      session = {
        doc => {},
      }
    end

    last_modified session[:updated_at]
    content_type :json
    session.doc.to_json
  end

  put '/session' do
    request.body.rewind

    data = JSON.parse(request.body.read)

    # build the change
    change = {
      :$set => {
        :updated_at => Time.now,
      }
    }
    data['delta'].each do |k,v|
      change[:$set]["doc.#{k}"] = v
    end

    $db.sessions.update({}, change, :upsert => true)

    # grab the latest session
    session = $db.sessions.find_one()

    $faye.publish('/session', session.doc)

    last_modified session[:updated_at]
    content_type :json
    session.doc.to_json
  end


  #
  # assets
  #

  before '/assets/:type/*' do
    # reject the request if not a supported asset type
    halt 404 unless ASSET_TYPES.include?(params[:type])
  end

  get '/assets/:type' do
#    if respond_to?("asset_#{type}_get".to_sym)
#      send("asset_#{type}_get")
#    end

    type = params[:type]

    assets = $db.assets[type].find({}).to_a

    content_type :json
    assets.to_json
  end

  def asset_texture_post(record)
    id      = hash_filename(params[:file][:filename])
    id_str  = "%08x" % id

    # load posted image data
    source_image = TextureCompiler::Image.load(params[:file][:tempfile])

    # try to automatically figure out a good semantic from the filename
    filename = params[:file][:filename]
    semantic = 'diffuse'
    if filename =~ /normal/i or filename =~ /_n$/i
      semantic = 'normalmap'
    elsif filename =~ /specular/i or filename =~ /_s$/i
      semantic = 'specmap'
    end

    # compile to DXT
    compiled_image = source_image.to_dxt

    # extract image metadata and build document for the db
    record.merge!({
      :_id      => id_str,
      :filename => params[:file][:filename],
      :doc      => {
        :height   => image.height,
        :width    => image.width,
        :semantic => semantic,
      }
    })

    fs = Mongo::GridFileSystem.new($db)

    # store the source and compiled images in mongo
    fs.open("/texture/#{id_str}/source", 'w') do |f|
      f.write(params[:file][:tempfile])
    end

    # store the compiled image data
    fs.open("/texture/#{id_str}/osx", 'w') do |f|
      f.write(compiled_image.surface)
    end
  end

  # usage:
  #   texture:  curl -X POST -F 'file=@filename' http://host/assets/texture
  post '/assets/texture' do
    pp params
    file      = params[:file]
    filename  = params[:file][:filename]
    filename  = params[:file][:filename]
    id        = hash_filename(filename)
    id_str    = "%08x" % id
    now       = Time.now

    # store source file
    src_file_id = nil
    fs = Mongo::GridFileSystem.new($db)
    fs.open("/texture/#{id_str}/source", 'w') do |f|
      src_file_id = f.files_id
      f.write(params[:file][:tempfile])
    end

    # store the metadata
    record = {
      :_id        => id_str,
      :created_at => now,
      :filename   => filename,
      :type       => 'texture',
      :updated_at => now,
      :version    => 0,
      :status     => {
        :source     => false,
        :osx_x64    => false,
      },
      :file_ids   => {
        :source     => src_file_id,
      },
      :metadata   => {
      },
    }
    $db.assets[:texture].insert(record)

    $faye.publish("/assets/texture/#{id_str}", record)
    Resque.enqueue(TextureSourceProcessorJob, id_str, 0)

    content_type :json
    record.to_json
  end

  post '/assets/:type' do
    type = params[:type]

    # build up some default record data
    now = Time.now
    record = {
      :created_at => now,
      :updated_at => now,
    }

    # allow custom asset-type processing first
    if respond_to?("asset_#{type}_post".to_sym)
      send("asset_#{type}_post".to_sym, record)
    else
      # expect JSON document data in the params
      id      = hash_filename(data[:filename])
      id_str  = "%08x" % id
      record.merge!({
        :_id      => id_str,
        :filname  => data[:filename],
        :doc      => JSON.parse(params[:doc]),
      })
    end

    $db.assets[type].insert(record)

#    etag doc[:digest]
    content_type :json
    record.to_json
  end

  # usage: curl -X POST -f 'file=@filename' http://host/assets/:type/:id/bulk
  post '/assets/:type/:id/bulk' do
    type      = params[:type]
    id        = params[:id]
    data      = params[:file][:tempfile]

    grid = Mongo::Grid.new($db, "fs_#{type}")
    grid.delete(id)
    grid.put(data, :_id => id)

    200
  end

  # deprecated - not async safe since the metadata may not match the file id
  get '/assets/:type/:id/bulk' do
    type      = params[:type]
    id        = params[:id]

    query = {
      :_id => id,
    }
    asset = $db.assets[type].find_one(query)
    if asset.nil?
      halt 404
    end
    puts asset.to_json
    pp asset

    file_id = asset.file_ids.osx_x64
    puts ">------"
    pp file_id
    puts "<------"

    # get the bulk file and read it
    grid = Mongo::Grid.new($db)
    io = grid.get(file_id)

    content_type 'application/octet-stream'
    io.read.to_s
  end

  get '/assets/:type/:id' do
    type  = params[:type]
    id    = params[:id]

    query = {
      :_id => id,
    }
    result = $db.assets[type].find_one(query)
    if result.nil?
      halt 404
    end

#    etag result.digest
    last_modified result[:updated_at]
    content_type :json
    result.to_json
  end

  get '/bulk/:file_id' do
    file_id = BSON::ObjectId.from_string(params[:file_id])

    # get the bulk file and read it
    grid = Mongo::Grid.new($db)
    io = grid.get(file_id)

    content_type 'application/octet-stream'
    io.read.to_s
  end

  put '/assets/:type/:id' do
    request.body.rewind

    type  = params[:type]
    id    = params[:id]
    data  = JSON.parse(request.body.read)

    query = {
      :_id      => id
    }

    # build the change
    update = {
      :$set => {
        :updated_at         => Time.now,
        'status.osx_x64'    => false,
      },
      :$unset => {
        'file_ids.osx_x64'  => 1,
      },
      :$inc => {
        :version            => 1,
      },
    }

    update[:$set].merge!(data['delta'].flatten_with_path('metadata'))
    result = $db.assets[type].find_and_modify(
      :query  => query,
      :update => update,
      :new    => true
    )
    puts result

    $faye.publish("/assets/texture/#{id}", result)

    # request recompilation
    Resque.enqueue(TextureCompilerJob, id, :osx_x64, result.version)

    last_modified result[:updated_at]
    content_type :json
    result.to_json
  end



  private
    def hash_filename(filename)
      return Hashes::Murmur3.hash(filename.downcase.gsub('\\', '/'), 0xdeadbeef)
    end
end
