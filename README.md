# Image Feature Detector for Neuromorphic hardware

## Compiling the project
To configure the project building and binary linking IFD makes use of CMake.
This program is tested on Windows 10, Ubuntu 18.04 /18.10 and MacOS 10.14.3

#### Resolving dependencies

* Installing nm500 SDK : NM500 is AI hardware which is based on neuromorphic technology. It can be used in many ways with many feature extracting algorithms.
For installation, please refer readme at lib foler.

* 
[Installing Qt](http://doc.qt.io/qt-5/linux.html): on Linux and Debian-based distributions you don't need to compile Qt. The easiest and fastest way to install Qt is with `apt-get` (details on the link).

* 
[Installing OpenCV](http://opencv.org/quickstart.html): due to some OpenCV copyrighted code, the modules containing SIFT and SURF detectors aren't available on Debian repositories like the rest of [OpenCV modules](https://packages.debian.org/search?keywords=opencv). This means you will have to compile OpenCV libraries by yourself. By and large you will need to download the main [OpenCV modules](https://github.com/Itseez/opencv) plus the [OpenCV contrib](https://github.com/Itseez/opencv_contrib) ones (the code containing SIFT and SURF detectors), and when you are about to compile the main OpenCV modules set the `OPENCV_EXTRA_MODULES_PATH` CMake variable to `<opencv_contrib>/modules`. Better explained [here](https://github.com/Itseez/opencv_contrib).

* 
Installing VLC : For rtsp stream, it requires libvlc.

For linux, 3 packages will require. 
  $ sudo apt install libvlc-dev libvlccore-dev vlc

For windows, just download and install latest vlc which contains SDK. 
it is packaged as 7zip file and SDK folders are included. Other zip, exe packages do not contains SDK folder.

For Mac, there is no binary package yet. however, you can compile with instructions from https://wiki.videolan.org/OSXCompile/

On Unix machines, once you have these two dependencies solved, from the command shell just run:
$ cd <imagefeaturedetector_root_folder>
$ mkdir build
$ cd build
$ cmake ..
$ make

On windows, For Qt and OpenCV, vcpkg (https://github.com/Microsoft/vcpkg) is the most convenient way.

$ cd <imagefeaturedetector_root_folder>
$ mkdir msbuild
$ cd msbuild
$ cmake  .. -G"Visual Studio 15 2017 Win64" "-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
$ cmake --build . --config release

For any enquiries, contact wfms123    a t  g m a i   l