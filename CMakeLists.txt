project(boyd)

cmake_minimum_required(VERSION 2.8)

add_definitions(-std=c++11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake") 
find_package(Daylite REQUIRED)
find_package(BsonBind REQUIRED)

if(WIN32)
  find_path(OPENCV_DIR lib PATHS "$ENV{OPENCV_DIR}" NO_DEFAULT_PATH)
  find_path(OPENCV_INCLUDE_DIR opencv2 PATHS "${OPENCV_DIR}/../../include" NO_DEFAULT_PATH)
  set(OPENCV_LIBRARIES
      ${OPENCV_DIR}/lib/opencv_core300.lib
      ${OPENCV_DIR}/lib/opencv_videoio300.lib)
else()
  set(OPENCV_LIBRARIES opencv_core opencv_videoio)
endif()

include_directories(${CMAKE_BINARY_DIR} ${DAYLITE_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIR})

add_bson_binds(BSON_BIND_INCLUDES image_color.bsonbind settings.bsonbind)
add_executable(boyd boyd.cpp ${BSON_BIND_INCLUDES})

target_link_libraries(boyd ${DAYLITE_LIBRARIES} ${OPENCV_LIBRARIES})