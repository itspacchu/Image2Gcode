cmake_minimum_required(VERSION 3.0)
project("Image2Gcode")

find_package (OpenCV 4.0.0 REQUIRED)
include_directories ("/usr/include/opencv4/")

add_executable(Image2Gcode main.cpp gcode.cpp)
target_link_libraries( Image2Gcode ${OpenCV_LIBS} )