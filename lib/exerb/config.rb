
#==============================================================================#
# $Id: config.rb,v 1.23 2008/06/13 23:54:45 arton Exp $
#==============================================================================#

require 'rbconfig'

#==============================================================================#

module Exerb
  ver = RUBY_VERSION.gsub('.','')

  # Search directories of a core.
  # If running exerb on exerb, Add self path to the search directories of a core.
  CORE_PATH = [
    (File.dirname(ExerbRuntime.filepath) if defined?(ExerbRuntime)),
    ENV['EXERBCORE'],
    File.expand_path('../../../data/exerb', __FILE__),
    File.join(RbConfig::CONFIG['datadir'], 'exerb')
  ].compact

  # Name definitions of a core.
  CORE_NAME = {
    'cui'    => "ruby#{ver}c.exc",
#   'cuid'   => "ruby#{ver}cd.exc",
    'cuirt'  => "ruby#{ver}crt.exc",
#   'cuirtd' => "ruby#{ver}crtd.exc",
    'gui'    => "ruby#{ver}g.exc",
#   'guid'   => "ruby#{ver}gd.exc",
    'guirt'  => "ruby#{ver}grt.exc",
#   'guirtd' => "ruby#{ver}grtd.exc",
  }

  # Descriptions of a core.
  CORE_DESC = {
    'ruby#{ver}c.exc'   => 'ruby #{RUBY_VERSION} / CUI / stand-alone',
    'ruby#{ver}g.exc'   => 'ruby #{RUBY_VERSION} / GUI / stand-alone',
    'ruby#{ver}crt.exc' => 'ruby #{RUBY_VERSION} / CUI / using runtime-library',
    'ruby#{ver}grt.exc' => 'ruby #{RUBY_VERSION} / GUI / using runtime-library',
  }

end # Exerb

#==============================================================================#

# Load a configuration file of Exerb Core Collection if found that.
configcc = File.join(File.dirname(__FILE__), 'configcc.rb')
if File.exist?(configcc)
  require(configcc)
end

#==============================================================================#
#==============================================================================#
