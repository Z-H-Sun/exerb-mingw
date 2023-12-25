# Exerb

Packaged ruby source files and required other (*.rb *.so *.dll) to a .exe file.

## Installation

0. Check your Ruby installation
    * Make sure you have `rake`; otherwise, run `gem install rake`
      * For Ruby < 1.9, you need a lower version rake that is compatible by specifying `-v 10.5.0` (highest version compatible with Ruby 1.8.7)
    * For the current version, it requires that your Ruby installation should have a static runtime library before this gem's installation. Go to the folder `<Your_Ruby_Path>/lib`, and check if there exists the required file which has a name like `lib<Platform>-ruby<Version_Number>-static.a`. Especially note the word `static` in the filename; the file named `lib<Platform>-ruby<Version_Number>-dll.a` is for dynamic linking and therefore NOT useful. If this is not the case, you can
      * Compile a static Ruby library on your own. If I have time, I may work on adding some guidance in this project in the future.
      * Visit RubyInstaller's [archived versions here](https://github.com/oneclick/rubyinstaller/releases) (Note: Not "RubyInstaller2" where they dropped the static library distribution) or [a clearer list here](https://rubyinstaller.org/downloads/archives/) (but you need to limit to those with version prior to 2.4)

1. install RubyInstaller's DevKit

    * Refer to [RubyInstaller's wiki page](https://github.com/oneclick/rubyinstaller/wiki/Development-Kit/25db58138bb49410ef9d7b695dbd1e8384b47871) on how to add the DevKit to your Ruby installation

2. install gem from local

    ```bat
    git clone https://github.com/Z-H-Sun/exerb-mingw.git
    cd exerb-mingw
    gem build exerb.gemspec
    gem install exerb --local --verbose && del *.gem
    ```
    * If you don't have `git`, it is also OK to [download the current snapshot here](https://github.com/Z-H-Sun/exerb-mingw/archive/refs/heads/master.zip) and extract from the archive instead
    * If you just want to compile the codes into binaries only without installing (i.e., copying the relevant files to the Ruby path), instead of `gem install ...`, run `rake generate`

## Updates and Known Bugs

* Recover compatibility with Ruby 1.8
* Now compatible with 64-bit Ruby. However, the generated 64-bit executables cannot dynamically load libraries (e.g., `Win32API.new(...)`), which will cause Segmentation Fault. There are likely more differences between 32-bit and 64-bit images that need further attention.
* The RT (run-time) cores won't work for Ruby >= 1.9; don't use them (This is because `Rakefile` fails to map correct exports of the exerb runtime DLL; in the future, `dumpbin` might be worth trying although it is proprietary?)
* Not yet tested with Ruby >= 2.4. (RubyInstaller stopped providing the static library; I have not yet tried compiling it manually)
* Summary of undocumented features since Exerb 5.4
  * `-c` option will compress the codes
  * `type: compiled-script` in .exy file will compile and squentialize the Ruby code file (only works for Ruby >= 1.9)
  * ...

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

1. Fork it ( https://github.com/Z-H-Sun/exerb-mingw/fork )
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create a new Pull Request

## License

Copyright (C) 2023 Z-H-Sun

Copyright (C) 2014 windwiny

Copyright (C) 2001-2006 加藤勇也

This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the [LICENSE file](/LICENSE) for more details.

The file `vendor/mkexports.rb` is copied from the source code repository of Ruby, which is copyrighted free software developed by Yukihiro Matsumoto, licensed by the 2-clause BSD License.

The binary form of this library may contain code from zlib (Copyright (C) 1995-2023 Jean-loup Gailly and Mark Adler).
