begin
  require 'devkit'
  raise LoadError unless ENV['RI_DEVKIT'] # need RubyInstaller's DevKit, not devkit gem.
rescue LoadError
  msg = "You need to install the devkit to compile exerb-mingw.\n"
  msg << "  http://rubyinstaller.org/add-ons/devkit/"
  abort msg
end
require 'rbconfig'
require 'ostruct'
require 'rake/clean'

raise('Unknown Ruby version: '+RUBY_VERSION) unless RUBY_VERSION =~ /(\d+)\.(\d+)/
HIGH_VER_RUBY = (($1.to_i) > 1 or ($2.to_i) > 8) # Ruby >= 1.9
if HIGH_VER_RUBY
# Include vendored scripts
$LOAD_PATH.unshift File.expand_path("../vendor", __FILE__)
require "mkexports"

EXERB_CFLAGS = "-DRUBY19 -DRUBY19_COMPILED_CODE"
else EXERB_CFLAGS = '' end # add flags only for Ruby >= 1.9

C = OpenStruct.new
c = RbConfig::CONFIG
c['CFLAGS'] = ' -Wall -std=gnu99 -O3 -g -Wextra -Wno-unused-parameter -Wno-parentheses -Wno-long-long -Wno-missing-field-initializers -Werror=pointer-arith -Werror=write-strings '
C.arch = c["arch"]
C.cc = "#{c['CC'] || 'gcc'}"
C.cflags = "#{c['CFLAGS'] || '-Os'}"
C.xcflags = "#{c['XCFLAGS'] || '-DRUBY_EXPORT'}"
C.exerb_cflags = EXERB_CFLAGS
C.cppflags = c['CPPFLAGS']
if c['rubyhdrdir']
  C.incflags = "-I#{c['rubyhdrdir']}/#{c['arch']} -I#{c['rubyhdrdir']}" if c['rubyhdrdir']
else
  C.incflags = "-I#{c['archdir']}"
end
C.ldflags = "-L#{c['libdir']}"
C.xldflags = "#{c['XLDFLAGS'] || '-Wl,--stack=0x02000000,--wrap=rb_require_safe,--wrap=rb_require'}"
C.rubylib = "#{c['LIBRUBYARG_STATIC']}"
C.libs = "#{c['LIBS']}"
C.ver = RUBY_VERSION.gsub('.','')

def make_resource(target, source, type)
  file target => source do
    mkdir_p File.dirname(target)
    sh "windres -D#{type} #{source} #{target}"
  end
end

def compile_c(target, source)
  cmdline = "#{C.cc} #{C.cflags} #{C.xcflags} #{C.cppflags} #{C.incflags} -c -o #{target} #{source}"
  file target => source do
    mkdir_p File.dirname(target)
    sh cmdline
  end
end

def link_cpp(target, options)
  sources = options[:sources]
  cc = 'gcc'
  cflags = "#{C.cflags} #{C.xcflags} #{C.exerb_cflags} #{C.cppflags} #{C.incflags}"
  ldflags = "#{C.ldflags} #{C.xldflags}"
  dllflags = options[:isdll] ? "-shared" : ""
  guiflags = options[:gui] ? "-mwindows" : ""
  dldflags = options[:isdll] ? "-Wl,--enable-auto-image-base,--enable-auto-import,--export-all" : ""
  impflags = options[:implib] ? "-Wl,--out-implib=#{options[:implib]}" : ""
  defflags = options[:def] ? "-Wl,--output-def=#{options[:def]}" : "" # revert commit 884f462
  rubylib = (options[:rubylib] || C.rubylib)
  libs = C.libs
  cmdline = "#{cc} #{cflags} #{ldflags} #{dllflags} #{guiflags} #{dldflags} #{impflags} #{defflags} -s -o #{target} #{sources.join(' ')} #{rubylib} #{libs}"
  file target => sources + Array(options[:dependencies]) do
    mkdir_p File.dirname(options[:implib]) if options[:implib]
    mkdir_p File.dirname(target)
    sh cmdline
    sh "strip -R .reloc #{target}" unless options[:isdll]
  end
  file options[:implib] => target if options[:implib]
  file options[:def] => target if options[:def]
end

# FIXME: is not mapping all the exports
def make_def_proxy(target, source, proxy)
  file target => source do
    mkdir_p File.dirname(target)
    File.open(target, "w") do |out|
      File.open(source, "r").each_line do |line|
        case line
        when /\A\s*(LIBRARY|EXPORTS)/
          out.puts line
        when /\A(\s*)(\w+@\w+)=(rb_w32_\w+@.*)\Z/
          out.puts "#{$1}#{$2} = #{proxy}.#{$3}"
        when /\A(\s*)(\w+)=(rb_w32_\w+)\Z/
          out.puts "#{$1}#{$2} = #{proxy}.#{$3}"
  		when /\A(\s*)(\w+@\w+)(\s+@.*)\Z/
    	  out.puts "#{$1}#{$2} = #{proxy}.#{$2}#{$3}"
        when /\A(\s*)(\w+)(\s+@.*)\Z/
          out.puts "#{$1}#{$2} = #{proxy}.#{$2}#{$3}"
        else
          out.puts line
        end
      end
    end
  end
end

def make_def(target, source_lib)
  file target => [source_lib] do
    mkdir_p File.dirname(target)

    puts "generating #{target}"
    exports = Exports.extract([source_lib])
    Exports.output(target) { |f| f.puts(*exports) }
  end
end

def make_config(target, libruby_name, libruby_so)
  file target do
    puts "generating header #{target}"
    File.open(target, "w") do |f|
      f.write <<-EOH.gsub(/^\s+/, '')
        #ifndef _EXERB_CONFIG_H_
        #define _EXERB_CONFIG_H_

        #define EXERB_LIBRUBY_NAME "#{libruby_name}"
        #define EXERB_LIBRUBY_SO   "#{libruby_so}"

        #endif /* _EXERB_CONFIG_H_ */
      EOH
    end
  end
  CLEAN.include target
end

# Ruby 1.9.1 doesn't have SyncEnumerator
# This function is inspired by Python's zip
def zip(*enums)
  r = block_given? ? nil : []
  len = enums.collect { |x| x.size }.max
  len.times do |i|
    val = enums.collect { |x| x[i] }
    if block_given?
      yield val
    else
      r << val
    end
  end
  r
end

exerb_dll_base        = "exerb61"
exerb_config_header   = "src/exerb/config.h"
file_resource_rc      = "src/exerb/resource.rc"
file_resource_dll_o   = "tmp/resource_dll.o"
file_resource_cui_o   = "tmp/resource_cui.o"
file_resource_gui_o   = "tmp/resource_gui.o"
file_exerb_def        = "tmp/#{exerb_dll_base}.def"
file_exerb_lib        = "tmp/#{exerb_dll_base}.dll.a"
file_exerb_rt_def     = "tmp/#{exerb_dll_base}_rt.def"
file_exerb_dll        = "data/exerb/#{exerb_dll_base}.dll"
file_ruby_cui         = "data/exerb/ruby#{C.ver}c.exc"
file_ruby_cui_rt      = "data/exerb/ruby#{C.ver}crt.exc"
file_ruby_gui         = "data/exerb/ruby#{C.ver}g.exc"
file_ruby_gui_rt      = "data/exerb/ruby#{C.ver}grt.exc"

lib_sources = Dir["src/exerb/{exerb,module,utility,patch}.c"]
dll_sources = [file_resource_dll_o]
cui_sources = ["src/exerb/cui.c", file_resource_cui_o]
gui_sources = ["src/exerb/gui.c", file_resource_gui_o]

make_resource file_resource_dll_o, file_resource_rc, "RUNTIME"

make_config exerb_config_header, c["RUBY_SO_NAME"], c["LIBRUBY_SO"]

objs_rt = [file_exerb_lib]
objs = lib_sources + [file_exerb_def]
if HIGH_VER_RUBY # FIXME: All rt cores won't work properly for Ruby >= 1.9. Since commit 884f462, file_exerb_rt_def have been removed from linking sources, just in order to pass compilation
make_def file_exerb_def, File.join(c["libdir"], c["LIBRUBY_A"])
link_cpp file_exerb_dll, :sources => (dll_sources + objs), :dependencies => exerb_config_header, :isdll => true, :implib => file_exerb_lib
else # revert commit 884f462 for Ruby 1.8
link_cpp file_exerb_dll, :sources => (dll_sources + lib_sources), :dependencies => exerb_config_header, :isdll => true, :def => file_exerb_def, :implib => file_exerb_lib
make_def_proxy file_exerb_rt_def, file_exerb_def, exerb_dll_base
objs_rt.push file_exerb_rt_def
end

make_resource file_resource_cui_o, file_resource_rc, "CUI"
link_cpp file_ruby_cui, :sources => (cui_sources + objs), :dependencies => exerb_config_header
link_cpp file_ruby_cui_rt, :sources => (cui_sources + objs_rt), :rubylib => "", :dependencies => exerb_config_header

make_resource file_resource_gui_o, file_resource_rc, "GUI"
link_cpp file_ruby_gui, :sources => (gui_sources + objs), :gui => true, :dependencies => exerb_config_header
link_cpp file_ruby_gui_rt, :sources => (gui_sources + objs_rt), :rubylib => "", :gui => true, :dependencies => exerb_config_header

task :rm_tmp do
  FileUtils.rm_rf('tmp') if File.directory?('tmp')
end

desc "generate cui and gui exa data"
task :generate => [
  file_ruby_cui,
  file_ruby_cui_rt,
  file_ruby_gui,
  file_ruby_gui_rt,
  :rm_tmp
]

desc "show infomation"
task :default do
  puts "Please use 'gem' to build and install gem."
  puts "  This Rakefile used by generate data."
end

CLEAN.include('tmp')
CLOBBER.include('data')
