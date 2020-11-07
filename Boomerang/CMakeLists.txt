cmake_minimum_required (VERSION 3.8)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")



project( Boomerang )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )


add_executable(Boomerang main.cpp)
target_link_libraries( Boomerang ${OpenCV_LIBS} )