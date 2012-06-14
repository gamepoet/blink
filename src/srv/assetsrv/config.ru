require 'faye'
require 'resque/server'
require './app'

use Faye::RackAdapter, :mount => '/faye', :timeout => 25

map '/' do
  run App
end

map '/resque' do
  run Resque::Server
end
