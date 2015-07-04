project(boyd)

cmake_minimum_required(VERSION 2.8)

add_definitions(-std=c++11)

function(add_bson_bind PATH_ INCLUDES)
  get_filename_component(NAME ${PATH_} NAME_WE)
  add_custom_command(OUTPUT ${NAME}.hpp COMMAND bson_bind ${PATH_} ${CMAKE_BINARY_DIR}/${NAME}.hpp WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  set(${INCLUDES} ${${INCLUDES}} ${NAME}.hpp PARENT_SCOPE)
endfunction()

include_directories(${CMAKE_BINARY_DIR})

################################################################################
#   Find libbson                                                               #
################################################################################

if(WIN32)
  find_file(LIBBSON_DLL libbson-1.0.dll PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/bin" NO_DEFAULT_PATH)
  find_path(LIBBSON_INCLUDE_DIR bson.h PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/include/libbson-1.0" NO_DEFAULT_PATH)
  find_library(LIBBSON_LIBRARY bson-1.0.lib PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/lib" NO_DEFAULT_PATH)
elseif(APPLE)
  set(LIBBSON_INCLUDE_DIR "/usr/local/include/libbson-1.0")
  set(LIBBSON_LIBRARY "bson-1.0")
endif()

include_directories(${LIBBSON_INCLUDE_DIR})
add_bson_bind(image_color.bsonbind BSON_BIND_INCLUDES)
add_bson_bind(settings.bsonbind BSON_BIND_INCLUDES)
add_executable(boyd boyd.cpp ${BSON_BIND_INCLUDES})

target_link_libraries(boyd ${LIBBSON_LIBRARY} daylite opencv_core opencv_videoio)