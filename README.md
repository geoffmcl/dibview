# DibView (dv32) Project

#### History (Brief)

This project started as a 16-bit, windows application, circa 1990, yes, 16-bit, with 20-bit memory addressing...

Then circa 1999, converted fully to WIN32, hence the exe name `dv32.exe`, with the latest build, using MSVC16 2019, in x64 `64-bits`. Be warned, it is quite a messy source, due to its ancestry... and the sporatic growth over the years...

Originally intended to support other image formats, like `JPEG`, `GIF`, `TIFF`, etc, this has been abandoned, at this time. There are just so many other great, `free`, tools available, like [ImageMagick][1], [XnView][2], and on and on... Eventally, it seemed silly to pursue this path...

Since `DibView` primary supports the Microsoft bitmap, [BMP][3] image format, or DIB, Device Independant Bitmap, and uses the Microsoft Window GUI, it is an **ONLY FOR WINDOWS** application.

This repository replaces the `ZIP` sources offered by my web page - [Image Viewer][4] - where you can read some more historical information...

  [1]: https://imagemagick.org/index.php
  [2]: https://www.xnview.com/en/
  [3]: https://en.wikipedia.org/wiki/BMP_file_format
  [4]: http://geoffair.org/ms/dib_view.htm
  
#### Licence

GNU GPL v.2 - see [LICENSE.txt](LICENSE.txt) - as far as possible.

Note there are some unused `JPEG` sources, which have their own license.

#### Build

To compile `DibView` from source, you need [CMake](http://www.cmake.org/install/) installed. This allows you to generate the build system of your choice. Run `cmake --help` to see the list of `Generators` available, on your system.

If your chosen `generator` is installed, then to build `DibView` do -

```
   cd build
   cmake .. [options]
   cmake --build . --config Release
```
   
There are some `option` listed in the `CMakeLists.txt` file, but at this time most do **NOT** compile! 

Use `-DUSE_STATIC_RUNTIME=ON`, to use the dynamic windows runtime - make a smaller executable.
   
In Windows, it does **not** make much sense to actually `install` the resultant `dv32.exe`. You can create a Windows `shortcut` to the exe, to run it... or copy it to a folder already existing in your `PATH` environment variable, if you want to load it from the command line.

Have **FUN** - Geoff - 20200622

; eof
