project(boyd)

cmake_minimum_required(VERSION 2.8)

add_definitions(-std=c++11)
add_definitions(-g -pg)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake") 
find_package(Daylite REQUIRED)
find_package(BsonBind REQUIRED)
find_package(OpenCV REQUIRED)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(BSONBINDS_DIR ${CMAKE_SOURCE_DIR}/bson_binds)

file(GLOB SRCS ${SRC_DIR}/*.cpp)
#file(GLOB INCLUDES ${INCLUDE_DIR}/*.hpp)
file(GLOB BSONBINDS ${BSONBINDS_DIR}/*.bsonbind)

include_directories(${CMAKE_BINARY_DIR} ${INCLUDE_DIR} ${DAYLITE_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS})

add_bson_binds(BSON_BIND_INCLUDES ${BSONBINDS})
add_executable(boyd ${SRCS} ${BSON_BIND_INCLUDES})
add_executable(test test/test.cpp)

target_link_libraries(boyd ${DAYLITE_LIBRARIES} ${OPENCV_LIBRARIES})
target_link_libraries(test ${DAYLITE_LIBRARIES} ${OPENCV_LIBRARIES})