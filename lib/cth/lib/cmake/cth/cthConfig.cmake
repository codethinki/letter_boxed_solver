
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was cthConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include(CMakeFindDependencyMacro)

set(_cth_all_components "cth;os;coro;net")
set(_cth_closure_cth "")
set(_cth_closure_os "cth")
set(_cth_closure_coro "cth;os")
set(_cth_closure_net "cth;os")

set(_cth_requested_original "${cth_FIND_COMPONENTS}")
set(_cth_requested_normalized "")
foreach(_cth_c IN LISTS _cth_requested_original)
    string(REPLACE "::" "_" _cth_cn "${_cth_c}")
    list(APPEND _cth_requested_normalized "${_cth_cn}")
endforeach()

if(NOT _cth_requested_normalized)
    set(_cth_requested_normalized "cth;os;coro;net")
    set(_cth_requested_original "cth;os;coro;net")
endif()

list(LENGTH _cth_requested_normalized _cth_num_requested)
if(_cth_num_requested GREATER 0)
    math(EXPR _cth_last_idx "${_cth_num_requested} - 1")
    foreach(_cth_i RANGE 0 ${_cth_last_idx})
        list(GET _cth_requested_normalized ${_cth_i} _cth_comp)
        list(GET _cth_requested_original ${_cth_i} _cth_orig)

        list(FIND _cth_all_components "${_cth_comp}" _cth_known_idx)
        if(_cth_known_idx EQUAL -1)
            set(cth_${_cth_comp}_FOUND FALSE)
            if(cth_FIND_REQUIRED_${_cth_orig})
                set(cth_FOUND FALSE)
                string(APPEND cth_NOT_FOUND_MESSAGE "Requested component '${_cth_orig}' is not a known cth component.\n")
            endif()
        endif()
    endforeach()
endif()

set(_cth_load "")
foreach(_cth_c IN LISTS _cth_requested_normalized)
    list(FIND _cth_all_components "${_cth_c}" _cth_known_idx)
    if(NOT _cth_known_idx EQUAL -1)
        list(APPEND _cth_load "${_cth_c}")
        list(APPEND _cth_load ${_cth_closure_${_cth_c}})
    endif()
endforeach()
if(_cth_load)
    list(REMOVE_DUPLICATES _cth_load)
endif()

list(FIND _cth_load "cth" _cth_idx)
if(NOT _cth_idx EQUAL -1)
    include("${CMAKE_CURRENT_LIST_DIR}/cth_cth-targets.cmake")
    set(cth_cth_FOUND TRUE)
endif()

list(FIND _cth_load "os" _cth_idx)
if(NOT _cth_idx EQUAL -1)
    include("${CMAKE_CURRENT_LIST_DIR}/cth_os-targets.cmake")
    set(cth_os_FOUND TRUE)
endif()

list(FIND _cth_load "coro" _cth_idx)
if(NOT _cth_idx EQUAL -1)

block(SCOPE_FOR VARIABLES)
    set(CMAKE_MESSAGE_LOG_LEVEL ERROR)
    find_package(asio CONFIG QUIET)
    if(NOT asio_FOUND)
        set(MSG " component 'cth_coro' dependency missing: find_package(asio CONFIG REQUIRED) failed")
        message(FATAL_ERROR "${MSG}")
    endif()
endblock()
find_dependency(asio CONFIG REQUIRED)


    include("${CMAKE_CURRENT_LIST_DIR}/cth_coro-targets.cmake")
    set(cth_coro_FOUND TRUE)
endif()

list(FIND _cth_load "net" _cth_idx)
if(NOT _cth_idx EQUAL -1)

block(SCOPE_FOR VARIABLES)
    set(CMAKE_MESSAGE_LOG_LEVEL ERROR)
    find_package(libhv CONFIG QUIET)
    if(NOT libhv_FOUND)
        set(MSG " component 'cth_net' dependency missing: find_package(libhv CONFIG REQUIRED) failed")
        message(FATAL_ERROR "${MSG}")
    endif()
endblock()
find_dependency(libhv CONFIG REQUIRED)


    include("${CMAKE_CURRENT_LIST_DIR}/cth_net-targets.cmake")
    set(cth_net_FOUND TRUE)
endif()

if(_cth_num_requested GREATER 0)
    foreach(_cth_i RANGE 0 ${_cth_last_idx})
        list(GET _cth_requested_normalized ${_cth_i} _cth_comp)
        list(GET _cth_requested_original ${_cth_i} _cth_orig)
        if(NOT _cth_comp STREQUAL _cth_orig)
            set(cth_${_cth_orig}_FOUND "${cth_${_cth_comp}_FOUND}")
        endif()
    endforeach()
endif()

check_required_components(cth)
