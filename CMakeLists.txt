## http://cmake.org/runningcmake
## http://doc.qt.io/qt-5/cmake-manual.html

cmake_minimum_required(VERSION 3.12)

project(imagefeaturedetector)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules
                      ${CMAKE_MODULE_PATH})


SET(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin) # http://stackoverflow.com/questions/18826789/cmake-output-build-directory
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
#set(AUTOUIC_OPTIONS -include ${CMAKE_CURRENT_SOURCE_DIR}/src/videolabelview.hpp)
set(CMAKE_AUTOUIC ON)
#set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,modernize-*)
#include(cppcheck.cmake)

## http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
if (WIN32)
    set(OpenCV_STATIC OFF)
    set(OpenCV SHARED ON)
#    set(OpenCV_DIR "C:/Users/kspark/opencv")
    set(LIBVLC_INCLUDE_DIR "C:/vlc-winsdk/sdk/include")
    set(LIBVLC_LIBRARY_DIR "C:/vlc-winsdk/sdk/lib")
    set(LIBVLC_LIBRARY "C:/vlc-winsdk/sdk/lib/libvlc.lib")
    set(LIBVLCCORE_LIBRARY "C:/vlc-winsdk/sdk/lib/libvlccore.lib")        
	set(NM500LLIBDIR_X64 "X:/NM500-SDK-V1/Library/x64")
	set(NeuroMemLibs  ${NM500LLIBDIR_X64}/NeuroMemEngine.lib ${NM500LLIBDIR_X64}/CyAPI.lib ${NM500LLIBDIR_X64}/cyusbserial.lib)


elseif(NOT WIN32)
    set(LIBVLC_INCLUDE_DIR "/usr/local/include")
    set(LIBVLC_LIBRARY_DIR "/usr/local/lib")
    set(LIBVLC_LIBRARY ${LIBVLC_LIBRARY_DIR}/libvlc.dylib)
    set(LIBVLCCORE_LIBRARY ${LIBVLC_LIBRARY_DIR}/libvlccore.dylib)
endif (WIN32)
find_package(OpenCV COMPONENTS opencv_core opencv_imgproc opencv_highgui opencv_objdetect opencv_tracking opencv_xfeatures2d REQUIRED)
include_directories(${LIBVLC_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})

file(GLOB ImageFeatureDetector_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
file(GLOB ImageFeatureDetector_UI ${CMAKE_CURRENT_SOURCE_DIR}/ui/*)

find_package(CppCheck)
# Add the "make analysis" target
if( CPPCHECK_FOUND )
    add_custom_target(analysis)
   # ADD_DEPENDENCIES(analysis ${ALL_ANALYSIS_TARGETS} ${ImageFeatureDetector_SRC})
    set_target_properties(analysis PROPERTIES EXCLUDE_FROM_ALL TRUE)
    message("analysis analysis targets are ${ALL_ANALYSIS_TARGETS}")
endif()

add_executable(imagefeaturedetector ${ImageFeatureDetector_SRC} ${ImageFeatureDetector_UI} resources.qrc)
if (WIN32)
    target_link_libraries(imagefeaturedetector Qt5::Core Qt5::Widgets ${OpenCV_LIBS} ${LIBVLC_LIBRARY} ${LIBVLCCORE_LIBRARY} ${NeuroMemLibs})
elseif(NOT WIN32)
    target_link_libraries(imagefeaturedetector Qt5::Core Qt5::Widgets ${OpenCV_LIBS} ${LIBVLC_LIBRARY} ${LIBVLCCORE_LIBRARY})
endif(WIN32)

