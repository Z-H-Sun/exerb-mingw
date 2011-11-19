
#==============================================================================#
# $Id: icon_file.rb,v 1.5 2010/06/26 03:13:36 arton Exp $
#==============================================================================#

require 'exerb/win32/struct/icon_header'
require 'exerb/win32/struct/icon_dir_entry'

#==============================================================================#

module Exerb
  module Win32
  end # Win32
end # Exerb

#==============================================================================#

class Exerb::Win32::IconFile

  def initialize
    @entries = []
  end

  attr_accessor :entries

  def self.read(input)
    case input
    when IO     then return self.new.read(input)
    when String then return File.open(input, 'rb') { |file| self.read(file) }
    else raise(ArgumentError, "input must be IO or String object")
    end
  end

  def read(io)
    base = io.pos
    icon_header = Exerb::Win32::Struct::IconHeader.read(io)

    @entries = (1..icon_header.count).collect {
      Exerb::Win32::Struct::IconDirEntry.read(io)
    }.collect { |icon_dir_entry|
      io.seek(base + icon_dir_entry.image_offset)
      icon_image_header = Exerb::Win32::Struct::IconImageHeader.read(io)
      io.seek(icon_image_header.position)

      entry = Exerb::Win32::IconFile::Entry.new
      entry.width     = icon_dir_entry.width
      entry.height    = icon_dir_entry.height
      entry.bit_count = icon_image_header.bit_count
      entry.value     = io.read(icon_dir_entry.bytes_in_res)
      if entry.width == 0
        entry.bit_count = icon_dir_entry.bit_count
        entry.width = entry.height = 256
      end
      entry
    }

    return self
  end

end # Exerb::Win32::IconFile

#==============================================================================#

class Exerb::Win32::IconFile::Entry

  def initialize
    @width     = nil
    @height    = nil
    @bit_count = nil
    @value     = nil
  end

  attr_accessor :width, :height, :bit_count, :value

end # Exerb::Win32::IconFile::Entry

#==============================================================================#
#==============================================================================#
