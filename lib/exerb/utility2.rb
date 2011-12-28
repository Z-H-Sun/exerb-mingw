
#==============================================================================#
# $Id: utility2.rb,v 1.2 2005/04/17 15:17:26 yuya Exp $
#==============================================================================#

module Exerb
end # Exerb

#==============================================================================#

module Exerb::Utility2

  def self.loaded_features(reject_list = [])
    reject_list << File.expand_path(__FILE__)

    if RUBY_VERSION == "1.9.3"
      $LOADED_FEATURES << 'enc/trans/utf_16_32.so'
      $LOADED_FEATURES << 'enc/trans/single_byte.so'
      $LOADED_FEATURES.uniq!
    end

    return $LOADED_FEATURES.collect { |filename|
      case filename.downcase
      when /\.rb$/o  then type = "script"
      when /\.so$/o  then type = "extension-library"
      when /\.dll$/o then type = "extension-library"
      end
      [type, filename]
    }.collect { |type, filename|
      if File.exist?(filename)
        [type, require_name(filename), filename]
      else
        $LOAD_PATH.collect { |dirpath|
          [type, filename, File.join(dirpath, filename)]
        }.find { |type, filename, filepath|
          File.exist?(filepath)
        }
      end
    }.compact.reject { |type, filename, filepath|
      type.nil?
    }.reject { |type, filename, filepath|
      reject_list.index(File.expand_path(filepath))
    }
  end

  def self.require_name(filename)
    path = $LOAD_PATH.find { |path| filename.include?(path) }
    if path
      # remove both the path and the platform from the filename
      return filename.gsub("#{path}/", "").gsub("#{RUBY_PLATFORM}/", "")
    end

    filename
  end
end # Exerb::Utility2

#==============================================================================#
#==============================================================================#
