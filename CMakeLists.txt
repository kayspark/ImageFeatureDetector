## http://cmake.org/runningcmake
## http://doc.qt.io/qt-5/cmake-manual.html

cmake_minimum_required(VERSION 3.11)

project(imagefeaturedetector)

SET(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin) # http://stackoverflow.com/questions/18826789/cmake-output-build-directory
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON) # Not working. To much time invested on this.
#set(CMAKE_AUTOUIC ON) # Not useful. It causes troubles with subdirectories, and with the linking order. Better use qt5_wrap_ui()
include_directories(${CMAKE_SOURCE_DIR}/build/imagefeaturedetector_autogen)

## http://stackoverflow.com/questions/11783932/how-to-add-linker-or-compile-flag-in-cmake-file
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lz")


## http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
if (win32)
    set(OpenCV_STATIC OFF)
    set(OpenCV SHARED ON)
    set(OpenCV_DIR "C:/Users/kspark/opencv")
    set(LIBVLC_INCLUDE_DIR "C:/vlc-winsdk/sdk/include")
    set(LIBVLC_LIBRARY_DIR "C:/vlc-winsdk/sdk/lib")
    set(LIBVLC_LIBRARY "C:/vlc-winsdk/sdk/lib/libvlc.lib")
    set(LIBVLCCORE_LIBRARY "C:/vlc-winsdk/sdk/lib/libvlccore.lib")
else (win32)
    set(LIBVLC_INCLUDE_DIR "/usr/local/include")
    set(LIBVLC_LIBRARY_DIR "/usr/local/lib")
    set(LIBVLC_LIBRARY ${LIBVLC_LIBRARY_DIR}/libvlc.dylib)
    set(LIBVLCCORE_LIBRARY ${LIBVLC_LIBRARY_DIR}/libvlccore.dylib)
    set(OpenCV_DIR "/usr/local/Cellar/opencv/4.0.1/lib/cmake/opencv4")
endif (win32)
find_package(OpenCV COMPONENTS opencv_core opencv_imgproc opencv_highgui opencv_objdetect opencv_tracking opencv_xfeatures2d REQUIRED)
include_directories(. ${LIBVLC_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})
# find_package(Threads REQUIRED) # http://stackoverflow.com/questions/1620918/cmake-and-libpthread (if not 'libpthread.so.0: DSO missing from command line' error will be thrown at linking time)
# find_package(ZLIB REQUIRED) # http://stackoverflow.com/questions/6173915/need-help-trying-to-make-cmake-find-third-party-libraries


## http://stackoverflow.com/questions/3201154/cmake-automatically-add-all-files-in-a-folder-to-a-target
file(GLOB ImageFeatureDetector_SRC ${CMAKE_SOURCE_DIR}/src/*)
file(GLOB ImageFeatureDetector_UI ${CMAKE_SOURCE_DIR}/ui/*)
# file(GLOB ImageFeatureDetector_A ${CMAKE_SOURCE_DIR}/libs/*) # http://stackoverflow.com/questions/14077611/how-do-i-tell-cmake-to-link-in-a-static-library-in-the-source-directory


## http://stackoverflow.com/questions/29968264/linking-and-uic-order-in-a-cmake-qt-project
qt5_wrap_ui(ImageFeatureDetector_UI_H ${ImageFeatureDetector_UI})

add_executable(imagefeaturedetector ${ImageFeatureDetector_SRC} ${ImageFeatureDetector_UI_H})

target_link_libraries(imagefeaturedetector Qt5::Core Qt5::Widgets ${OpenCV_LIBS} ${LIBVLC_LIBRARY} ${LIBVLCCORE_LIBRARY})
# target_link_libraries(imagefeaturedetector Qt5::Core Qt5::Widgets ${ImageFeatureDetector_A} ${CMAKE_THREAD_LIBS_INIT} -lz -ldl)


## http://stackoverflow.com/questions/13429656/cmake-how-to-copy-contents-of-a-directory-into-build-directory-after-make
add_custom_command(TARGET imagefeaturedetector POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/icons $<TARGET_FILE_DIR:imagefeaturedetector>/icons)

include(clang-tidy.cmake)
## http://www.cmake.org/Wiki/CMake:Install_Commands
## Icons won't be shown because the loading path is relative to the command shell path at the moment of calling ./imagefeaturedetector
# install (TARGETS imagefeaturedetector DESTINATION bin)
# file(GLOB icons ${CMAKE_SOURCE_DIR}/icons/*)
# install (FILES ${icons} DESTINATION bin/icons)



