# change the default general compilation options below
# alternatively, pass these flags by `set EXERB_GENERAL_CFLAGS=...`
EXERB_GENERAL_CFLAGS = '-std=gnu99 -O3 -g -Wall -Wextra -Wno-unused-parameter -Wno-parentheses -Wno-long-long -Wno-missing-field-initializers -Werror=pointer-arith -Werror=write-strings'
# specify the C compiler by `set EXERB_C_COMPILER=...`
# if C compiler is not defined in either the env var nor RbConfig::CONFIG['CC'], below will be the fallback value
EXERB_C_COMPILER = 'gcc'

begin
  require 'devkit'
  raise LoadError unless ENV['RI_DEVKIT'] # need RubyInstaller's DevKit, not devkit gem.
rescue LoadError
  msg = "You need to install the devkit to compile exerb-mingw.\n"
  msg << "https://github.com/oneclick/rubyinstaller/releases/tag/devkit-4.7.2"
  abort msg
end
require 'rbconfig'
require 'ostruct'
require 'rake/clean'

C = OpenStruct.new
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

def link_c(target, options)
  sources = options[:sources]
  cflags = "#{C.cflags} #{C.xcflags} #{C.exerb_cflags} #{C.cppflags} #{C.incflags}"
  ldflags = "#{C.ldflags} #{C.xldflags}"
  dllflags = options[:isdll] ? "-shared" : ""
  guiflags = options[:gui] ? "-mwindows" : ""
  dldflags = options[:isdll] ? "-Wl,--enable-auto-image-base,--enable-auto-import,--export-all" : ""
  impflags = options[:implib] ? "-Wl,--out-implib=#{options[:implib]}" : ""
  defflags = options[:def] ? "-Wl,--output-def=#{options[:def]}" : "" # revert commit 884f462
  rubylib = (options[:rubylib] || C.rubylib)
  libs = C.libs
  cmdline = "#{C.cc} #{cflags} #{ldflags} #{dllflags} #{guiflags} #{dldflags} #{impflags} #{defflags} -s -o #{target} #{sources.join(' ')} #{rubylib} #{libs}"
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

    puts "generating '#{target}'"
    exports = Exports.extract([source_lib])
    Exports.output(target) { |f| f.puts(*exports) }
  end
end

def make_config(target, libruby_name, libruby_so)
  file target do
    puts "generating header '#{target}'"
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

task :config do
  raise('Unknown Ruby version: '+RUBY_VERSION) unless RUBY_VERSION =~ /(\d+)\.(\d+)/
  HIGH_VER_RUBY = (($1.to_i) > 1 or ($2.to_i) > 8) # Ruby >= 1.9
  if HIGH_VER_RUBY
    # Include vendored scripts
    $LOAD_PATH.unshift File.expand_path("../vendor", __FILE__)
    require "mkexports"
    EXERB_CFLAGS = "-DRUBY19 -DRUBY19_COMPILED_CODE"
  else
    EXERB_CFLAGS = '' # add flags only for Ruby >= 1.9
  end

  c = RbConfig::CONFIG
  C.arch = c["arch"]
  C.cc = ENV['EXERB_C_COMPILER'] || c['CC'] || 'gcc'
  C.cflags = ENV['EXERB_GENERAL_CFLAGS'] || EXERB_GENERAL_CFLAGS
  C.xcflags = c['XCFLAGS'] || '-DRUBY_EXPORT'
  C.exerb_cflags = EXERB_CFLAGS
  C.cppflags = c['CPPFLAGS']
  C.incflags = c['rubyhdrdir'] ? "-I#{c['rubyhdrdir']}/#{c['arch']} -I#{c['rubyhdrdir']}" : "-I#{c['archdir']}"
  C.ldflags = "-L#{c['libdir']}"
  C.xldflags = "#{c['XLDFLAGS'] || '-Wl,--stack=0x02000000'} -Wl,--wrap=rb_require_safe,--wrap=rb_require"
  C.rubylib = c['LIBRUBYARG_STATIC']
  C.libs = c['LIBS']

  lib_sources = Dir["src/exerb/{exerb,module,utility,patch}.c"]
  dll_sources = [file_resource_dll_o]
  cui_sources = ["src/exerb/cui.c", file_resource_cui_o]
  gui_sources = ["src/exerb/gui.c", file_resource_gui_o]

  module StaticZlibTest # this namespace definition is important for low Ruby versions < 2.0!
# Otherwise, `mkmf`'s `rm_f` (imported from `fileutils`) will have conflict with `Rake::FileUtils` or `Rake::FileUtilsExe`'s `rm_f`
# Though this bug has been fixed in higher Ruby versions (by introducing a `MakeMakefile` module and operate explicitly under that namespace)
    if ENV['EXERB_NO_STATIC_ZLIB'] # this whole thing can be skipped by `set EXERB_NO_STATIC_ZLIB=1`
      puts "\nYou have chosen not to link static Zlib library for code compressing."
    else # check if there is static zlib in mingw
# dynamic ones should theoretically work, but you need to provide the dll alongside with your binary distribution, which is undesired
# thus the name `zdll` is not checked below as the name indicates dynamic lib
      require "mkmf" # will generate 'mkmf.log' when testing; should add it to .gitignore
      $extmk = true # test using static ruby lib: in `link_command` in 'mkmf.rb': `librubyarg = $extmk ? $LIBRUBYARG_STATIC : $LIBRUBYARG`
      puts "\nChecking if there is a static Zlib library..."
      C.zlib_libname = %w'z libz zlib1 zlib'.find {|z| have_library(z+' -static', 'deflateReset')} if have_header('zlib.h') # test using `-static` option (alternatively, set `c['LIBARG'] = '-static -l%s'`)
    end
  end
  if C.zlib_libname
    C.exerb_cflags += ' -DHAVE_STATIC_ZLIB'
    C.rubylib += ' -Wl,-Bstatic -l' + C.zlib_libname
    lib_sources.push 'vendor/zlib.c'
  else
    puts 'No static Zlib library will be linked. But don`t worry!'
    print "The code compressing function is still available, and it will be achieved by dynamic loading.\n\n"
  end
  # TODO: should not export symbols for zlib (in `-Wl,--export-all`; this will not be a problem for HIGH_VER_RUBY as a different method will be used to generate def file)
  # TODO: the current zlib.c (Ruby wrapper) only works for Ruby 1.8; should update to higher version?
  # TODO: should not include zlib.so in the exy file list for this static zlib version, so should make something like a config file? (see 'lib/exerb/utility2.rb')

  make_resource file_resource_dll_o, file_resource_rc, "RUNTIME"

  make_config exerb_config_header, c["RUBY_SO_NAME"], c["LIBRUBY_SO"]

  objs_rt = [file_exerb_lib]
  objs = lib_sources + [file_exerb_def]
  if HIGH_VER_RUBY # FIXME: All rt cores won't work properly for Ruby >= 1.9. Since commit 884f462, file_exerb_rt_def have been removed from linking sources, just in order to pass compilation
    make_def file_exerb_def, File.join(c["libdir"], c["LIBRUBY_A"])
    link_c file_exerb_dll, :sources => (dll_sources + objs), :dependencies => exerb_config_header, :isdll => true, :implib => file_exerb_lib
  else # revert commit 884f462 for Ruby 1.8
    link_c file_exerb_dll, :sources => (dll_sources + lib_sources), :dependencies => exerb_config_header, :isdll => true, :def => file_exerb_def, :implib => file_exerb_lib
    make_def_proxy file_exerb_rt_def, file_exerb_def, exerb_dll_base
    objs_rt.push file_exerb_rt_def
  end

  make_resource file_resource_cui_o, file_resource_rc, "CUI"
  link_c file_ruby_cui, :sources => (cui_sources + objs), :dependencies => exerb_config_header
  link_c file_ruby_cui_rt, :sources => (cui_sources + objs_rt), :rubylib => "", :dependencies => exerb_config_header

  make_resource file_resource_gui_o, file_resource_rc, "GUI"
  link_c file_ruby_gui, :sources => (gui_sources + objs), :gui => true, :dependencies => exerb_config_header
  link_c file_ruby_gui_rt, :sources => (gui_sources + objs_rt), :rubylib => "", :gui => true, :dependencies => exerb_config_header
end

task :rm_tmp do
  puts "removing folder 'tmp'"
  FileUtils.rm_rf('tmp') if File.directory?('tmp')
end
task :rm_bin do
  puts "removing folder 'data'"
  FileUtils.rm_rf('data') if File.directory?('data')
end
task :compile => [
  file_ruby_cui,
  file_ruby_cui_rt,
  file_ruby_gui,
  file_ruby_gui_rt] do
  print "
==================================================
Finished compiling exc data files for Ruby#{C.ver}!#{HIGH_VER_RUBY ?
"\n\nNote: Known bug: The runtime cores will not work,
i.e., do not use the 'cuirt' and 'guirt' cores!" : ''}
==================================================\n\n"
end

desc "generate cui and gui exc data"
task :generate => [
  :generate_no_clean,
  :rm_tmp
]

desc "same as above but w/o cleaning up temp output files"
task :generate_no_clean => [
  :rm_bin,
  :config,
  :compile
]

desc "show infomation"
task :default do
  puts "\nAvailable `rake` tasks:"
  system 'rake --tasks'
  puts "\nThis Rakefile is used to generate ONLY the binary files in the 'data' folder."
  puts "If you are sure this is really what your want, run `rake generate`. Otherwise:"
  puts "\nPlease use 'gem' to build and install this gem. Run the following command:"
  puts "`gem build exerb.gemspec && gem install exerb --local --verbose`"
end

CLEAN.include(exerb_config_header)
CLEAN.include('tmp')
CLOBBER.include('data')
