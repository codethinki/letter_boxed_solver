cth_glob_cpp(LBS_SOLVER_SRC "src")
cth_glob_cpp(LBS_SOLVER_INCL "incl")

add_library(lbs_solver STATIC ${LBS_SOLVER_SRC} ${LBS_SOLVER_INCL})
add_library(lbs_solver::solver ALIAS lbs_solver)

cth_target_enable_build_cache(lbs_solver OPTIONAL)


target_include_directories(
        lbs_solver
        PUBLIC incl
)


find_package(cth CONFIG REQUIRED)
target_link_libraries(lbs_solver PUBLIC cth::cth cth::win)

add_subdirectory(runner)

# tests
if(LBS_ENABLE_TESTS)
    add_subdirectory(tests)
endif()