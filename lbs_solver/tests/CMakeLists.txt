cth_glob_cpp(SOLVER_TESTS_SRC "src")

add_executable(lbs_solver_tests ${SOLVER_TESTS_SRC})
cth_add_resources(lbs_solver_tests "assets/")

cth_target_enable_build_cache(lbs_solver_tests OPTIONAL)
target_include_directories(
        lbs_solver_tests
        PRIVATE src
)


find_package(GTest CONFIG REQUIRED)

target_link_libraries(lbs_solver_tests PRIVATE GTest::gtest_main lbs_solver::solver)

include(GoogleTest)
gtest_discover_tests(lbs_solver_tests)