cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

find_package(gazebo REQUIRED)
list(APPEND CMAKE_CXX_FLAGS "${GAZEBO_CXX_FLAGS}")

find_library(GEOLIB Geographic [/usr/local/lib/])

message(${GEOLIB})

add_library(wp_loader_plugin SHARED wp_loader_plugin.cpp)
target_include_directories(wp_loader_plugin PRIVATE ${GAZEBO_INCLUDE_DIRS})
link_directories(${GAZEBO_LIBRARY_DIRS})
target_link_libraries(wp_loader_plugin PRIVATE ${GAZEBO_LIBRARIES} ${GEOLIB})