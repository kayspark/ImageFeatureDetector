## http://cmake.org/runningcmake
## http://doc.qt.io/qt-5/cmake-manual.html

cmake_minimum_required(VERSION 3.7)

project(imagefeaturedetector)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules
        ${CMAKE_MODULE_PATH})

SET(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR} / ui)
#set(AUTOUIC_OPTIONS - include ${CMAKE_CURRENT_SOURCE_DIR} / src / videolabelview.hpp)
#set(CMAKE_AUTOUIC ON)
#set(AUTOGEN_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR} / autogen)
#set(CMAKE_CXX_CLANG_TIDY clang - tidy - checks = -*, modernize - *)
#include(cppcheck.cmake)
## http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries
#find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
if (WIN32)
    set(OpenCV_STATIC OFF)
    set(OpenCV SHARED ON)

    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/win) # http://stackoverflow.com/questions/18826789/cmake-output-build-directory
#set(OpenCV_DIR "C:/Users/kspark/opencv")
    set(LIBVLC_INCLUDE_DIR "C:/vlc-winsdk/sdk/include")
    set(LIBVLC_LIBRARY_DIR "C:/vlc-winsdk/sdk/lib")
    set(LIBVLC_LIBRARY "C:/vlc-winsdk/sdk/lib/libvlc.lib")
    set(LIBVLCCORE_LIBRARY "C:/vlc-winsdk/sdk/lib/libvlccore.lib")
    set(NM500LLIBDIR_X64 "X:/NM500-SDK-V1/Library/x64")
    set(nmengine ${NM500LLIBDIR_X64}/NeuroMemEngine.lib ${NM500LLIBDIR_X64}/CyAPI.lib ${NM500LLIBDIR_X64}/cyusbserial.lib)
elseif (NOT WIN32)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin) # http://stackoverflow.com/questions/18826789/cmake-output-build-directory
    if (APPLE)
        set(LIBVLC_INCLUDE_DIR "/usr/local/include")
        set(LIBVLC_LIBRARY /usr/local/lib/libvlc.dylib)
        set(LIBVLCCORE_LIBRARY /usr/local/lib/libvlccore.dylib)
        set(nmengine /usr/local/lib/libnmengine.dylib)
    elseif (NOT APPLE)
        set(LIBVLC_INCLUDE_DIR "/usr/include")
#For Raspberry PI
#set(LIBVLC_LIBRARY_DIR "/usr/lib/arm-linux-gnueabihf")
#For Intel
        set(LIBVLC_LIBRARY_DIR "/usr/lib/x86_64-linux-gnu")
        set(LIBVLC_LIBRARY ${LIBVLC_LIBRARY_DIR}/libvlc.so)
        set(LIBVLCCORE_LIBRARY ${LIBVLC_LIBRARY_DIR}/libvlccore.so)
        set(nmengine /usr/local/lib/libnmengine.so)
    endif (APPLE)
endif (WIN32)
find_package(OpenCV COMPONENTS opencv_core opencv_imgproc opencv_highgui opencv_objdetect opencv_tracking opencv_xfeatures2d REQUIRED)
include_directories(${LIBVLC_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})

file(GLOB ImageFeatureDetector_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
#file(GLOB ImageFeatureDetector_UI ${CMAKE_CURRENT_SOURCE_DIR}/ui/*)

#find_package(CppCheck)
# Add the "make analysis" target
if (CPPCHECK_FOUND)
    add_custom_target(analysis)
    # ADD_DEPENDENCIES(analysis ${ImageFeatureDetector_SRC})
    set_target_properties(analysis PROPERTIES EXCLUDE_FROM_ALL TRUE)
    message("analysis analysis targets are ${ALL_ANALYSIS_TARGETS}")
endif ()

add_executable(imagefeaturedetector ${CMAKE_CURRENT_SOURCE_DIR}/src/ocv_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vlccap.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vlccap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/nm_detector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/nm_detector.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/nm_classifier.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/nm_classifier.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/motionCapture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/motionCapture.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/NeuroMemEngine.hpp
        )

target_link_libraries(imagefeaturedetector ${nmengine} ${OpenCV_LIBS} ${LIBVLC_LIBRARY} ${LIBVLCCORE_LIBRARY})

