# 0.) Create Source and CMakeFile
# 1.) mkdir build
# 2.) cd build
# 3.) cmake .. - Generting the build Files / Configure the project
# 4.) cmake --build .
# 5.) ./Excutable

cmake_minimum_required(VERSION 3.22)

project(cppBitmap VERSION 1.0.0 LANGUAGES C CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(Bitmap_Library STATIC
    Bitmap.cpp
    Bitmap.h
    BitmapFileHeader.h
    BitmapInfoHeader.h
    Mandelbrot.cpp
    Mandelbrot.h
    Zoom.h
    ZoomList.h
    ZoomList.cpp
    FractalCreator.h
    FractalCreator.cpp
    RGB.h
    RGB.cpp)

add_executable(bitmap main.cpp)


target_link_libraries(bitmap PRIVATE Bitmap_Library)

target_include_directories(bitmap PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})