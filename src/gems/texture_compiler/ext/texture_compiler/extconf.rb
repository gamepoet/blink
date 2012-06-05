require 'mkmf'

def check_libs(libs)
  libs.all? { |lib| have_library(lib) || abort("could not find library: #{lib}") }
end

def check_headers(headers)
  headers.all? { |header| have_header(header) || abort("could not find header: #{header}") }
end

dir_config(
  'tiff',
  ['/opt/local/include', Config::CONFIG['includedir']],
  ['/opt/local/lib', Config::CONFIG['libdir']]
  )

check_headers(%w(tiffio.h))
check_libs(%w(tiff))

create_makefile('texture_compiler/texture_compiler')
