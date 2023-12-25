=begin
This file was copied from Ruby's souce code repository (2.0.0-preview1).
3f539a5b5966d00f28bbae2617c006d93b491d17 is the commit hash when the file
was copied. Its latest version can be found in the following URL:
https://github.com/ruby/ruby/blob/master/win32/mkexports.rb

Ruby is copyrighted free software by Yukihiro Matsumoto <matz@netlab.jp>.
You can redistribute it and/or modify it under either the terms of the
2-clause BSD License:

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
=end

$name = $library = $description = nil

class Exports
  @@subclass = []
  def self.inherited(klass)
    @@subclass << [/#{klass.name.sub(/.*::/, '').downcase}/i, klass]
  end

  def self.create(*args, &block)
    platform = RUBY_PLATFORM
    pat, klass = @@subclass.find {|p, k| p =~ platform}
    unless klass
      raise ArgumentError, "unsupported platform: #{platform}"
    end
    klass.new(*args, &block)
  end

  def self.extract(objs, *rest)
    create(objs).exports(*rest)
  end

  def self.output(output = $output, &block)
    if output
      open(output, 'wb', &block)
    else
      yield STDOUT
    end
  end

  def initialize(objs)
    syms = {}
    winapis = {}
    syms["ruby_sysinit_real"] = "ruby_sysinit"
    each_export(objs) do |internal, export|
      syms[internal] = export
      winapis[$1] = internal if /^_?(rb_w32_\w+)(?:@\d+)?$/ =~ internal
    end
    # patch to use RbConfig information (rubyhdrdir)
    incdir = File.join(RbConfig::CONFIG["rubyhdrdir"], "ruby")
    read_substitution(incdir+"/win32.h", syms, winapis)
    read_substitution(incdir+"/subst.h", syms, winapis)
    syms["rb_w32_vsnprintf"] ||= "ruby_vsnprintf"
    syms["rb_w32_snprintf"] ||= "ruby_snprintf"
    @syms = syms
  end

  def read_substitution(header, syms, winapis)
    IO.foreach(header) do |line|
      if /^#define (\w+)\((.*?)\)\s+(?:\(void\))?(rb_w32_\w+)\((.*?)\)\s*$/ =~ line and
          $2.delete(" ") == $4.delete(" ")
        export, internal = $1, $3
        if syms[internal] or internal = winapis[internal]
          syms[forwarding(internal, export)] = internal
        end
      end
    end
  end

  def exports(name = $name, library = $library, description = $description)
    exports = []
    if name
      exports << "Name " + name
    elsif library
      exports << "Library " + library
    end
    exports << "Description " + description.dump if description
    exports << "VERSION #{RbConfig::CONFIG['MAJOR']}.#{RbConfig::CONFIG['MINOR']}"
    exports << "EXPORTS" << symbols()
    exports
  end

  private
  def forwarding(internal, export)
    internal.sub(/^[^@]+/, "\\1#{export}")
  end

  def each_export(objs)
  end

  def objdump(objs, &block)
    if objs.empty?
      $stdin.each_line(&block)
    else
      each_line(objs, &block)
    end
  end

  def symbols()
    @syms.sort.collect {|k, v| v ? v == true ? "#{k} DATA" : "#{k}=#{v}" : k}
  end
end

class Exports::Mswin < Exports
  def each_line(objs, &block)
    IO.popen(%w"dumpbin -symbols -exports" + objs) do |f|
      f.each(&block)
    end
  end

  def each_export(objs)
    noprefix = ($arch ||= nil and /^(sh|i\d86)/ !~ $arch)
    objs = objs.collect {|s| s.tr('/', '\\')}
    filetype = nil
    objdump(objs) do |l|
      if (filetype = l[/^File Type: (.+)/, 1])..(/^\f/ =~ l)
        case filetype
        when /OBJECT/, /LIBRARY/
          next if /^[[:xdigit:]]+ 0+ UNDEF / =~ l
          next unless /External/ =~ l
          next unless l.sub!(/.*?\s(\(\)\s+)?External\s+\|\s+/, '')
          is_data = !$1
          if noprefix or /^[@_]/ =~ l
            next if /(?!^)@.*@/ =~ l || /@[[:xdigit:]]{8,16}$/ =~ l ||
              /^_?(?:Init_|.*_threadptr_|DllMain\b)/ =~ l
            l.sub!(/^[@_]/, '') if /@\d+$/ !~ l
          elsif !l.sub!(/^(\S+) \([^@?\`\']*\)$/, '\1')
            next
          end
        when /DLL/
          next unless l.sub!(/^\s*\d+\s+[[:xdigit:]]+\s+[[:xdigit:]]+\s+/, '')
        else
          next
        end
        yield l.strip, is_data
      end
    end
    yield "strcasecmp", "msvcrt.stricmp"
    yield "strncasecmp", "msvcrt.strnicmp"
  end
end

class Exports::Cygwin < Exports
  def self.nm
    @@nm ||= RbConfig::CONFIG["NM"]
  end

  def exports(*)
    super()
  end

  def each_line(objs, &block)
    IO.foreach("|#{self.class.nm} --extern --defined #{objs.join(' ')}", &block)
  end

  def each_export(objs)
    symprefix = RbConfig::CONFIG["SYMBOL_PREFIX"]
    symprefix.strip! if symprefix
    re = /\s(?:(T)|[[:upper:]])\s#{symprefix}((?!Init_|.*_threadptr_|DllMain\b).*)$/
    objdump(objs) do |l|
      next if /@.*@/ =~ l
      yield $2, !$1 if re =~ l
    end
  end
end

class Exports::Mingw < Exports::Cygwin
  def each_export(objs)
    super
    yield "strcasecmp", "_stricmp"
    yield "strncasecmp", "_strnicmp"
  end
end

# END {
#   exports = Exports.extract(ARGV)
#   Exports.output {|f| f.puts(*exports)}
# }
