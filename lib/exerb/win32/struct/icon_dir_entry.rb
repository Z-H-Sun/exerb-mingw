
#==============================================================================#
# $Id: icon_dir_entry.rb,v 1.5 2010/06/26 03:13:36 arton Exp $
#==============================================================================#

require 'exerb/win32/struct/base'

#==============================================================================#

module Exerb
  module Win32
    module Struct
    end # Struct
  end # Win32
end # Exerb

#==============================================================================#

class Exerb::Win32::Struct::IconDirEntry < Exerb::Win32::Struct::Base

  FORMAT = 'CCCCSSLL'

  def initialize
    @width        = 0
    @height       = 0
    @color_count  = 0
    @reserved     = 0
    @planes       = 0
    @bit_count    = 0
    @bytes_in_res = 0
    @image_offset = 0
  end

  attr_accessor :width, :height, :color_count, :reserved, :planes, :bit_count, :bytes_in_res, :image_offset

  def pack
    return [@width, @height, @color_count, @reserved, @planes, @bit_count, @bytes_in_res, @image_offset].pack(FORMAT)
  end

  def unpack(bin)
    @width, @height, @color_count, @reserved, @planes, @bit_count, @bytes_in_res, @image_offset = bin.unpack(FORMAT)
    return self
  end

end # Exerb::Win32::Struct::IconDirEntry

#==============================================================================#

class Exerb::Win32::Struct::IconImageHeader < Exerb::Win32::Struct::Base

  FORMAT = 'LllSSLLllLL'

  def initialize
    # This struct is identical to BITMAPINFOHEADER.
    
    @_size           = 0
    @width           = 0
    @height          = 0
    @planes          = 0
    @bit_count       = 0
    @compression     = 0
    @size_image      = 0
    @xpels_per_meter = 0
    @ypels_per_meter = 0
    @clr_used        = 0
    @clr_important   = 0
  end

  attr_accessor :_size, :width, :height, :planes, :bit_count, :compression, :size_image, :xpels_per_meter, :ypels_per_meter, :clr_used, :clr_important

  def pack
    return [@_size, @width, @height, @planes, @bit_count, @compression, @size_image, @xpels_per_meter, @ypels_per_meter, @clr_used, @clr_important].pack(FORMAT)
  end

  def unpack(bin)
    @_size, @width, @height, @planes, @bit_count, @compression, @size_image, @xpels_per_meter, @ypels_per_meter, @clr_used, @clr_important = bin.unpack(FORMAT)
    return self
  end

end # Exerb::Win32::Struct::IconImageHeader

#==============================================================================#
#==============================================================================#
