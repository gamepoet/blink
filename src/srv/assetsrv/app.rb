require 'bundler/setup'
Bundler.require(:default)
$:.push(File.dirname(__FILE__) + '/lib')
require 'digest/sha1'
require 'blink/ext'

#
# db setup
#

$db = Mongo::Connection.new('localhost').db('blink')


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
      '/js/ember-config.js',
      '/js/vendor/ember-0.9.8.1.js',
      '/js/vendor/underscore-1.3.3.js',
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
    ]
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

    last_modified session[:updated_at]
    content_type :json
    session.doc.to_json
  end

  get '/assets/:type' do
    type = params[:type]

    assets = $db.assets[type].find({}).to_a

    content_type :json
    assets.to_json
  end

  post '/assets/:type' do
    request.body.rewind

    type      = params[:type]
    data      = JSON.parse(request.body.read)
    id        = "%08x" % hash_filename(data['filename'])
#    digest    = Digest::SHA1.hexdigest(data['doc'].to_json)

    doc      = {
      :_id        => id,
      :created_at => Time.now,
      :doc        => data['doc'],
#      :digest     => "sha1:#{digest}",
      :filename   => data['filename'],
    }
    $db.assets[type].insert(doc)

#    etag doc[:digest]
    content_type :json
    doc.to_json
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

  get '/assets/:type/:id/bulk' do
    type  = params[:type]
    id    = params[:id]

    grid = Mongo::Grid.new($db, "fs_#{type}")
    io = grid.get(id)

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
    change = {
      :$set => {
        :updated_at => Time.now,
      }
    }
    data['delta'].each do |k,v|
      change[:$set]["doc.#{k}"] = v
    end

    $db.assets[type].update(query, change)

    # return the latset data
    result = $db.assets[type].find_one(query)
    if result.nil?
      halt 404
    end

    last_modified result[:updated_at]
    content_type :json
    result.to_json
  end



  private
    def hash_filename(filename)
      return Hashes::Murmur3.hash(filename.downcase.gsub('\\', '/'), 0xdeadbeef)
    end
end
