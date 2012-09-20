require 'faye'
require 'resque/server'
require './app'

if ENV['RACK_ENV'] == 'development'
  use Rack::LiveReload
end
use Faye::RackAdapter, :mount => '/faye', :timeout => 25

map '/' do
  run App
end

map '/resque' do
  run Resque::Server
end
