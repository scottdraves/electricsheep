## Getting the Source ##
The project source is hosted at Source Forge, on the [[Sheep](http://sourceforge.net/projects/electricsheep/|Electric)] project page, and is available through the [[repository](http://sourceforge.net/svn/?group_id=68853|Subversion)]. If you browse the repository you will see that there are five folders under trunk. Their names and use are as follows:
**client - the reference client, for linux** client\_generic - new Windows client, also runs under Linux, intended to be portable
**client\_osx - MacOS X client** client\_windoze - old Windows client
**server - old version of the server
The long term goal is to make client\_generic a cross-platform build, which targets a number of platforms, including MacOS X which is currently being managed as a separate build.**

## Building on MS-Windows ##
```
Install TortoiseSVN.
Checkout ES svn to c:\esmsvc (https://electricsheep.svn.sourceforge.net/svnroot/electricsheep/trunk/client_generic)
Install Microsoft DirectX SDK (February 2010)
Install MSVC 2010 Express

Download wxWidgets 2.9.0 source. Unpack to c:\wxwidgets-2.9.0
Download latest libpng and zlib, replace all code files in c:\wxwidgets\src\libpng and c:\wxwidgets\src\zlib, respectively.
Modify .vcproj files to use /MT and /MTd instead of default /MD and /MDd (use notepad++ mass rename function - easiest)
use C:\wxWidgets-2.9.0\build\msw\wx_vc9.sln to build unicode release and unicode debug targets.
Copy wxzlib.lib and wxzlibd.lib to c:\wxwidgets\src\zlib to build libcurl later.

boost 1.43.0
open vcvars from start menu (visual studio 2010 command prompt)
cd c:\boost_1_43_0
Run bootstrap.bat to build bjam
Run bjam full build - requires ~5GB of space:
bjam --build-type=complete stage

Download libcurl and unpack it to folder c:\esmsvc\curl.

Edit file c:\esmsvc\curl\lib\makefile.vc9:
ZLIBLIBS = zlib.lib
replace with:
ZLIBLIBS = wxzlib.lib
ZLIBLIBSD = wxzlibd.lib

Modify target debug-zlib to use ZLIBLIBSD
Use visual studio 2010 command prompt and go to folder c:\esmsvc\curl\lib then type:
set ZLIB_PATH=c:\wxwidgets-2.9.0\src\zlib
nmake -f makefile.vc9 CFG=release-zlib RTLIBCFG=static
nmake -f makefile.vc9 CFG=debug-zlib RTLIBCFG=static

Build lua from svn using C:\esmsvc\lua5.1\mak.vs2005\lua5.1.sln
Build tinyxml from svn using C:\esmsvc\tinyXml\tinyxml.sln

Build ffmpeg under mingw/msys (see http://ffmpeg.arrozcru.org/forum/ for tips or if you want to build it another way)
Download necessary files from: http://sourceforge.net/projects/mingw/files/
Install MSYS-1.0.11.exe to c:\msys
Checkout ffmpeg svn to C:\msys\home\?your?login?\ffmpeg\svn
Download gnu binutils (2.20.1) and unpack+overwrite to C:\msys\mingw
Overwrite files using gcc bin, mingwrt dev and mingwrt dll package in c:\msys\mingw folder
Depending on current gcc version you will also need dlls for mpc, mpfr and gmp packages (4.5.0).

Run msys environment and go to ffmpeg folder:
cd ffmpeg

Create build folder and enter it:
cd build

Type:

../svn/configure --enable-memalign-hack --enable-gpl --enable-swscale --enable-w32threads --disable-encoders --disable-muxers --disable-demuxers --disable-doc --disable-ffplay --disable-ffserver --disable-network --enable-runtime-cpudetect --enable-version3 --enable-demuxer=avi --enable-hwaccel=h264_dxva2 --disable-decoders --enable-decoder=h264 --disable-parsers --disable-bsfs --enable-parser=h264 --cpu=i686 --disable-debug --disable-protocols --disable-filters --enable-protocol=file --target-os=mingw32
Use j4 if you have 4 cores:
make -j2
make install

Successful build creates C:\msys\local\include and C:\msys\local\lib
Create folder c:\esmsvc\ffmpeg\ffmpeg-static\lib and c:\esmsvc\ffmpeg\ffmpeg-static\include (copy C:\msys\local\include here).
Copy files from msys/mingw folders (C:\msys\mingw\lib, C:\msys\mingw\lib\gcc\mingw32\4.5.0): libgcc.a, libmingwex.a, libmoldname.a to c:\msys\local\lib folder.
Use .bat script (http://electricsheep.org/prepare_libs.bat) to create .lib files (it will copy libs to ffmpeg-static\lib folder automatically).

Now you can build C:\esmsvc\MSVC\electricsheep.sln (main program) and/or C:\esmsvc\MSVC\SettingsGUI\SettingsGUI.sln (settings gui)
Using installer from C:\esmsvc\InstallerMSVC will build installer package, after you copy appropriate files from DirectX SDK redist folder.
```

## Building on Linux ##
  * Development has moved over to client\_generic
  * The current client is part of debian and ubuntu: http://packages.debian.org/sid/electricsheep http://packages.ubuntu.com/lucid/electricsheep so you can use the source from their package systems.
  * Or use http://electricsheep.org/makesheep.sh to build from our SVN.
  * See [forum](http://community.electricsheep.org/node/271)

## Building on MacOS X ##
  * Open client\_generic/MacBuild/ElectricSheep-Xcode3.xcodeproj into Xcode 3.1.X (you need at least 10.5 for running it)
    * Or Open client\_generic/MacBuild/ElectricSheep.xcodeproj into Xcode 4 (this will not build for macos 10.4)
  * Target "ElectricSheepTestingApp" builds standalone application and "Screen Saver" one builds (wait for it...) screen saver module.
  * execute this command: "cp -r MacBuild/build/Debug/proximus.bundle ~/Library/Screen\ Savers/"
    * with Xcode 4, open the organizer to find the location of your derived data, and copy the proximus.bundle into place.
  * Choose Build from Build Menu
  * For Debug Active Build configuration the Screen Saver target will be built directly in ~/Library/Screen Savers folder so you can debug it directly
  * For Release Configuration it will be build directly in the build folder at the same level as a project
  * The all 32-bit targets are build using 10.4u SDK as a minimum system. 64-bit ones for 10.6 will be 10.6 minimum (64-bit screen saver exists only there).