require './app'
require 'resque/server'

map '/' do
  run App
end

map '/resque' do
  run Resque::Server
end
