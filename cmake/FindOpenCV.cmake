if(WIN32)
  find_path(OPENCV_DIR lib PATHS "$ENV{OPENCV_DIR}" NO_DEFAULT_PATH)
  find_path(OPENCV_INCLUDE_DIRS opencv2 PATHS "${OPENCV_DIR}/../../include" NO_DEFAULT_PATH)
  set(OPENCV_LIBRARIES
      ${OPENCV_DIR}/lib/opencv_core300.lib
      ${OPENCV_DIR}/lib/opencv_videoio300.lib)
else()
  set(OPENCV_LIBRARIES opencv_core opencv_videoio)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(OPENCV DEFAULT_MSG OPENCV_LIBRARIES OPENCV_INCLUDE_DIRS)

mark_as_advanced(OPENCV_INCLUDE_DIRS OPENCV_LIBRARIES)