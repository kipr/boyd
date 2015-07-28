if(WIN32)
  find_file(BSON_DLL libbson-1.0.dll PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/bin" NO_DEFAULT_PATH)
  find_path(BSON_INCLUDE_DIR bson.h PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/include/libbson-1.0" NO_DEFAULT_PATH)
  find_library(BSON_LIBRARY bson-1.0.lib PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/lib" NO_DEFAULT_PATH)
else()
  find_path(BSON_INCLUDE_DIR libbson-1.0/bson.h PATHS "${CMAKE_SOURCE_PREFIX}")
  find_library(BSON_LIBRARY NAMES bson-1.0 libbson-1.0 PATHS "${CMAKE_SOURCE_PREFIX}")
  
  set(BSON_INCLUDE_DIR "${BSON_INCLUDE_DIR}/libbson-1.0")
endif()

set(BSON_LIBRARIES ${BSON_LIBRARY})
set(BSON_INCLUDE_DIRS ${BSON_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(BSON DEFAULT_MSG BSON_LIBRARY BSON_INCLUDE_DIR)

mark_as_advanced(BSON_INCLUDE_DIR BSON_LIBRARY)