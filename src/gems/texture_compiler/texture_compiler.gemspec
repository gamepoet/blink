Gem::Specification.new do |s|
  s.name          = 'texture_compiler'
  s.version       = '0.0.1'
  s.summary       = 'Blink texture compiler.'
  s.description   = 'Compiles images to textures and manipulates their metadata.'
  s.authors       = ['Ben Scott']
  s.email         = ['gamepoet@gmail.com']
  s.homepage      = 'http://github.com/gamepoet/blink'
  s.files         = Dir.glob('lib/**/*.rb') +
                    Dir.glob('ext/**/*.{c,h,rb}')
  s.extensions    = ['ext/texture_compiler/extconf.rb']
end
