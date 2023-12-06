# Exerb

Packaged ruby source files and required other (*.rb *.so *.dll) to a .exe file.

## Installation


1. install RubyInstaller's DevKit

    from `http://rubyinstaller.org/add-ons/devkit` download and install  
    ```shell
    set DEVKITPATH = c:/rbmingw      # install path, like c:/rbmingw
    cd %DEVKITPATH%
    ruby dk.rb init
    notepad config.yml  # edit config file,  set ruby installed path  
    ruby dk.rb install
    ruby -e "require 'devkit'; puts ENV['RI_DEVKIT']" # test, ensure the path is %DEVKITPATH%  
    ```

2. install gem

    `gem install exerb`

## Updates and Known Bugs

Now compatible with 64-bit Ruby. However, the generated 64-bit executables cannot dynamically load libraries (e.g., `Win32API.new(...)`). There are likely more differences between 32-bit and 64-bit images that need further attention.

## Usage

1. generate one or more ruby source files do some thing and test it

    ruby r1.rb arg1 arg2 arg3

2. make exy file from ruby source file

    mkexy -- r1.rb arg1 arg2 arg3  # or  
    ruby -r exerb/mkexy r1.rb arg1 arg2 arg3

3. check r1.exy file, check every required files.  
   can be change `general.core` to `cui/gui`, add `resource` , `path` section  
   more demo old_*.exy in example/ and test/ directory
   
4. generate exe file

    exerb -v r1.exy

5. execute EXE file

    r1.exe arg1 arg2 arg3  # can receive some result as ruby r1.rb arg1 arg2 arg3

## Contributing

1. Fork it ( https://github.com/Z-H-Sun/exerb/fork )
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create a new Pull Request
