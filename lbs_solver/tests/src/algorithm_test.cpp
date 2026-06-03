//#include "solver/algorithm.hpp" // Ensure this includes your dfs implementation
//
//#include <gtest/gtest.h>
//
//#include <optional>
//#include <string>
//#include <vector>
//#include <span>
//
//
//namespace lbs {
//
//// ============================================================================
//// Generic Custom Graph Types for Testing
//// ============================================================================
//struct TestEdge;
//
//struct TestNode {
//    int id;
//    std::vector<TestEdge> edges{};
//};
//
//struct TestEdge {
//    int weight;
//    TestNode* to;
//};
//
//// Generic extraction callbacks
//constexpr auto get_edges = [](TestNode const& n) -> std::span<TestEdge const> {
//    return n.edges;
//};
//
//constexpr auto get_dest = [](TestEdge const& e) -> TestNode const& {
//    return *e.to;
//};
//
//
//// ============================================================================
//// Tests
//// ============================================================================
//
//TEST(BoundedDfsTest, TargetDepthZeroReturnsEmptyPath) {
//    TestNode root{42, {}};
//
//    auto next_state = [](int state, auto) -> std::optional<int> { return state; };
//    auto is_sol = [](TestNode const&, int state, auto path) {
//        return path.empty() && state == 0;
//    };
//
//    // Note: Using the 8-argument overload (no backtrack provided)
//    auto sols = dfs(
//        0,
//        10,
//        root,
//        get_edges,
//        get_dest,
//        0,
//        next_state,
//        is_sol
//    );
//
//    ASSERT_EQ(sols.size(), 1);
//    EXPECT_TRUE(sols[0].empty());
//}
//
//TEST(BoundedDfsTest, LinearPathAccumulatesState) {
//    // 0 =5=> 1 =10=> 2 =20=> 3
//    TestNode n3{3, {}};
//    TestNode n2{2, {{20, &n3}}};
//    TestNode n1{1, {{10, &n2}}};
//    TestNode root{0, {{5, &n1}}};
//
//    // State tracks the sum of traversed weights
//    auto next_state = [](int state, auto path) -> std::optional<int> {
//        return state + path.back()->weight;
//    };
//
//    // Solution is reaching node 3 with a specific accumulated state
//    auto is_sol = [&](TestNode const&, int state, auto path) {
//        return !path.empty() && path.back()->to == &n3 && state == 35; // 5 + 10 + 20
//    };
//
//    auto sols = dfs(
//        3,
//        10,
//        root,
//        get_edges,
//        get_dest,
//        0,
//        next_state,
//        is_sol
//    );
//
//    ASSERT_EQ(sols.size(), 1);
//    ASSERT_EQ(sols[0].size(), 3);
//    EXPECT_EQ(sols[0][0]->weight, 5);
//    EXPECT_EQ(sols[0][2]->weight, 20);
//}
//
//TEST(BoundedDfsTest, PrunesInvalidStatesUsingNullopt) {
//    TestNode target{99, {}};
//    TestNode bad_path{2, {{10, &target}}};
//    TestNode good_path{1, {{10, &target}}};
//
//    // The -1 weight should trigger a prune
//    TestNode root{0, {{-1, &bad_path}, {5, &good_path}}};
//
//    auto next_state = [](int state, auto path) -> std::optional<int> {
//        if(path.back()->weight < 0)
//            return std::nullopt; // PRUNE!
//        return state + path.back()->weight;
//    };
//
//    auto is_sol = [](TestNode const&, int, auto path) {
//        return !path.empty() && path.back()->to->id == 99;
//    };
//
//    auto sols = dfs(
//        2,
//        10,
//        root,
//        get_edges,
//        get_dest,
//        0,
//        next_state,
//        is_sol
//    );
//
//    // It should only find 1 solution, the good path.
//    ASSERT_EQ(sols.size(), 1);
//    ASSERT_EQ(sols[0].size(), 2);
//    EXPECT_EQ(sols[0][0]->weight, 5); // Asserts it went down 'good_path'
//}
//
//TEST(BoundedDfsTest, BacktrackCallbackIsTriggeredWithCorrectValues) {
//    // Root =10=> A =20=> B
//    TestNode nB{2, {}};
//    TestNode nA{1, {{20, &nB}}};
//    TestNode root{0, {{10, &nA}}};
//
//    auto next_state = [](int state, auto path) -> std::optional<int> {
//        return state + path.back()->weight;
//    };
//
//    auto is_sol = [](TestNode const&, int, auto path) {
//        return !path.empty() && path.back()->to->id == 2;
//    };
//
//    struct BtRecord {
//        int parent;
//        int popped;
//    };
//    std::vector<BtRecord> bt_records;
//
//    auto backtrack = [&](int parent, int popped, auto /*path*/) {
//        bt_records.push_back({parent, popped});
//    };
//
//    // target_depth = 2
//    auto sols = dfs(
//        2, 10,
//        root, get_edges, get_dest,
//        0, next_state, is_sol, backtrack
//    );
//
//    ASSERT_EQ(sols.size(), 1);
//
//    // Bounded DFS exactly depth 2:
//    ASSERT_EQ(bt_records.size(), 2);
//
//    // 1. Depth=2 checks node B, and then backtracks it
//    EXPECT_EQ(bt_records[0].parent, 10); // State at node A
//    EXPECT_EQ(bt_records[0].popped, 30); // State at node B
//
//    // 2. Depth=1 checks node A, and then backtracks it to Root
//    EXPECT_EQ(bt_records[1].parent, 0);  // State at Root
//    EXPECT_EQ(bt_records[1].popped, 10); // State at node A
//}
//
//TEST(BoundedDfsTest, RespectsMaxSolutionsEarlyExit) {
//    TestNode target1{1, {}};
//    TestNode target2{2, {}};
//    TestNode target3{3, {}};
//    TestNode root{0, {{10, &target1}, {20, &target2}, {30, &target3}}};
//
//    auto next_state = [](int state, auto) -> std::optional<int> { return state; };
//    auto is_sol = [](TestNode const&, int, auto path) { return !path.empty(); };
//
//    // Request strictly 2 solutions out of the 3 available
//    auto sols = dfs(
//        1,
//        2,
//        root,
//        get_edges,
//        get_dest,
//        0,
//        next_state,
//        is_sol
//    );
//
//    ASSERT_EQ(sols.size(), 2);
//    EXPECT_EQ(sols[0][0]->to->id, 1);
//    EXPECT_EQ(sols[1][0]->to->id, 2);
//}
//
//TEST(BoundedDfsTest, OnlyFindsSolutionsAtTargetDepth) {
//    // 0 -> 1 -> 2 -> 3
//    TestNode n3{3, {}};
//    TestNode n2{2, {{30, &n3}}};
//    TestNode n1{1, {{20, &n2}}};
//    TestNode root{0, {{10, &n1}}};
//
//    auto next_state = [](int state, auto) -> std::optional<int> { return state; };
//    auto is_sol = [&](TestNode const&, int, auto path) {
//        return !path.empty() && path.back()->to == &n3;
//    };
//
//    // Target depth 2. Node 3 is at depth 3. Should find nothing.
//    auto failed_sols_2 = dfs(
//        2,
//        10,
//        root,
//        get_edges,
//        get_dest,
//        0,
//        next_state,
//        is_sol
//    );
//    EXPECT_TRUE(failed_sols_2.empty());
//
//    // Target depth 4. Node 3 is at depth 3 and has no edges. 
//    // dfs evaluates solutions exactly at depth=4, so it should find nothing.
//    auto failed_sols_4 = dfs(
//        4,
//        10,
//        root,
//        get_edges,
//        get_dest,
//        0,
//        next_state,
//        is_sol
//    );
//    EXPECT_TRUE(failed_sols_4.empty());
//
//    // Expand depth exact to 3. Should succeed.
//    auto success_sols = dfs(
//        3,
//        10,
//        root,
//        get_edges,
//        get_dest,
//        0,
//        next_state,
//        is_sol
//    );
//    ASSERT_EQ(success_sols.size(), 1);
//    EXPECT_EQ(success_sols[0].size(), 3);
//}
//
//TEST(BoundedDfsTest, StateSupportsComplexMoveSemantics) {
//    // Uses std::string as State to prove that heavy/movable states 
//    // are correctly preserved, formatted, and destructed.
//    TestNode n2{2, {}};
//    TestNode n1{1, {{0, &n2}}};
//    TestNode root{0, {{0, &n1}}};
//
//    auto next_state = [](std::string const& prev, auto path) -> std::optional<std::string> {
//        return prev + "->" + std::to_string(path.back()->to->id);
//    };
//
//    auto is_sol = [](TestNode const&, std::string const& state, auto) {
//        return state == "Start->1->2";
//    };
//
//    auto sols = dfs(
//        2,
//        10,
//        root,
//        get_edges,
//        get_dest,
//        std::string("Start"),
//        next_state,
//        is_sol
//    );
//
//    ASSERT_EQ(sols.size(), 1);
//    EXPECT_EQ(sols[0].size(), 2);
//}
//
//} // namespace lbs