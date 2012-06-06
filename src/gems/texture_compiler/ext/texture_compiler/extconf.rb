require 'mkmf'

def check_libs(libs)
  libs.all? { |lib| have_library(lib) || abort("could not find library: #{lib}") }
end

def check_headers(headers)
  headers.all? { |header| have_header(header) || abort("could not find header: #{header}") }
end

dir_config(
  'tiff',
  ['/opt/local/include', RbConfig::CONFIG['includedir']],
  ['/opt/local/lib', RbConfig::CONFIG['libdir']]
  )

check_headers(%w(tiffio.h squish.h))
check_libs(%w(tiff squish))

# setup compile opts for c++
if RUBY_PLATFORM =~ /(mswin|mingw|bccwin)/
  USE_GCC = $1 == 'mingw'
else
  USE_GCC = true
end

case RUBY_PLATFORM
#when /mswin32/, /mingw32/, /bccwin32/
when /darwin/
  RbConfig::CONFIG['LDSHARED'] = "$(CXX) " + RbConfig::CONFIG['LDSHARED'].split[1..-1].join(' ')
end

create_makefile('texture_compiler/texture_compiler')
