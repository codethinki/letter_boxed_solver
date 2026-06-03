cth_glob_cpp(SOLVER_RUNNER_SRC "src")

add_executable(lbs_solver_runner ${SOLVER_RUNNER_SRC})
cth_add_resources(lbs_solver_runner "assets/")

cth_target_enable_build_cache(lbs_solver_runner OPTIONAL)
target_include_directories(
        lbs_solver_runner
        PRIVATE src 
)
target_link_libraries(lbs_solver_runner PRIVATE lbs_solver::solver)

# 1. Choose the correct flag for Clang / clang-cl
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # If the frontend is MSVC (clang-cl), we must use the /clang: prefix
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC" OR (WIN32 AND MSVC))
        set(CLANG_CONSTEXPR_FLAG "/clang:-fconstexpr-steps=100000000")
    else()
        # Standard Clang (0 = unlimited steps)
        set(CLANG_CONSTEXPR_FLAG "-fconstexpr-steps=10000000")
    endif()
endif()

# 2. Apply the flags to the target
target_compile_options(lbs_solver_runner PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:${CLANG_CONSTEXPR_FLAG}>
    $<$<CXX_COMPILER_ID:GNU>:-fconstexpr-ops-limit=100000000>
    $<$<CXX_COMPILER_ID:MSVC>:/constexpr:steps100000000>
)