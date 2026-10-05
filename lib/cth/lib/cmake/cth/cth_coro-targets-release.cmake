#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cth::coro" for configuration "Release"
set_property(TARGET cth::coro APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(cth::coro PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcth_coro.a"
  )

list(APPEND _cmake_import_check_targets cth::coro )
list(APPEND _cmake_import_check_files_for_cth::coro "${_IMPORT_PREFIX}/lib/libcth_coro.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
