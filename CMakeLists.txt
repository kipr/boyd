project(boyd)

cmake_minimum_required(VERSION 2.8)

add_definitions(-std=c++11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake") 
find_package(Daylite REQUIRED)
find_package(BsonBind REQUIRED)

include_directories(${CMAKE_BINARY_DIR} ${DAYLITE_INCLUDE_DIRS})

add_bson_bind(BSON_BIND_INCLUDES image_color.bsonbind settings.bsonbind)
add_executable(boyd boyd.cpp ${BSON_BIND_INCLUDES})

target_link_libraries(boyd ${DAYLITE_LIBRARIES} opencv_core opencv_videoio)