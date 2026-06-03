cmake_minimum_required(VERSION 4.1.1)
project(
  letter-boxed-solver
  LANGUAGES CXX
  VERSION 1.0.0
)

include(cth_setup_utilities)
include(cth_target_utilities)
include(cth_install_utilities)

cth_set_compiler_specifics()
cth_set_latest_c_cpp_standard()
set(CMAKE_CXX_SCAN_FOR_MODULES FALSE)

set(CMAKE_C_COMPILER_LAUNCHER "buildcache")
set(CMAKE_CXX_COMPILER_LAUNCHER "buildcache")


# subprojects
add_subdirectory(lbs_solver)

# test
if(LBS_ENABLE_TESTS)
	message(STATUS "enabled testing")
	enable_testing()
endif()

# formatting
cth_glob_cpp(
  FORMAT_FILES
  "lbs_solver"
)

#cth_add_uncrustify_target(uncrustify-format OPTIONAL ${FORMAT_FILES})
