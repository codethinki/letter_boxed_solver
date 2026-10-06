#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cth::net" for configuration "Release"
set_property(TARGET cth::net APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(cth::net PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcth_net.a"
  )

list(APPEND _cmake_import_check_targets cth::net )
list(APPEND _cmake_import_check_files_for_cth::net "${_IMPORT_PREFIX}/lib/libcth_net.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
