find_package(BSON REQUIRED)

if(WIN32)
  find_file(DAYLITE_DLL daylite.dll PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/bin" NO_DEFAULT_PATH)
  find_path(DAYLITE_INCLUDE_DIR daylite PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/include" NO_DEFAULT_PATH)
  find_library(DAYLITE_LIBRARY daylite.lib PATHS "${PROJECT_SOURCE_DIR}/../prefix/usr/lib" NO_DEFAULT_PATH)
else()
  find_path(DAYLITE_INCLUDE_DIR daylite/node.hpp PATHS "${CMAKE_SOURCE_PREFIX}")
  find_library(DAYLITE_LIBRARY NAMES daylite libdaylite PATHS "${CMAKE_SOURCE_PREFIX}")
endif()

set(DAYLITE_LIBRARIES ${DAYLITE_LIBRARY} ${BSON_LIBRARIES})
set(DAYLITE_INCLUDE_DIRS ${DAYLITE_INCLUDE_DIR} ${BSON_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Daylite DEFAULT_MSG DAYLITE_LIBRARY DAYLITE_INCLUDE_DIR)

mark_as_advanced(DAYLITE_INCLUDE_DIR DAYLITE_LIBRARY)