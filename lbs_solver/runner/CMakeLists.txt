cth_glob_cpp(SOLVER_RUNNER_SRC "src")

add_executable(lbs_solver_runner ${SOLVER_RUNNER_SRC})
cth_add_resources(lbs_solver_runner "assets/")

cth_target_enable_build_cache(lbs_solver_runner OPTIONAL)
target_include_directories(
        lbs_solver_runner
        PRIVATE src 
)
target_link_libraries(lbs_solver_runner PRIVATE lbs_solver::solver)

cth_target_set_constexpr_steps(lbs_solver_runner 1000000000)
