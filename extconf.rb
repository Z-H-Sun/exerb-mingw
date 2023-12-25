ARGV.unshift("generate")
require 'rubygems'
load Gem.bin_path('rake', 'rake')
ARGV.shift if ARGV.first == "generate"

## ------- fake extensions
STDERR.puts("==== fake extensions ====")
open('Makefile', 'w') { |f| f.write("clean:\ndefault:\ninstall:\n") }
