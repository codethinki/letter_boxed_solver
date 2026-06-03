#pragma once
#include <span>
#include <utility>
#include <vector>
#include <ranges>
#include <stop_token>


namespace lbs {
constexpr auto default_backtrack_fn = [](auto&&...) {};
constexpr auto default_stop_fn = []() { return false; };


/**
 * runs a dfs on a directed graph (does not exclude cycles)
 * @tparam Node node type
 * @tparam EdgesViewOfFn fn from node -> edges_view_rng
 * @tparam DestOfFn fn from edge -> node
 * @tparam State state type, movable
 * @tparam NextStateFn fn from (prev_state, edges_ptr_path) -> optional_like<state> (prunes on empty)
 * @tparam BacktrackFn fn from (parent_state, popped_state, edges_ptr_path)
 * @tparam StopFn fn from () -> bool
 * @return true if fully explored, false if stopped
 */
template<
    class Node,
    class EdgesViewOfFn,
    class DestOfFn,
    class State,
    class NextStateFn,
    class BacktrackFn,
    class StopFn
>
constexpr bool dfs(
    Node&& root,
    EdgesViewOfFn&& edges_view_of_fn,
    DestOfFn&& destination_of_fn,
    State&& initial_state,
    NextStateFn&& next_state_fn,
    BacktrackFn&& backtrack_fn,
    StopFn&& stop_fn
) {
    using edges_view_t = std::invoke_result_t<EdgesViewOfFn, Node>;
    using edge_t = std::remove_reference_t<std::ranges::range_reference_t<edges_view_t>>;
    using path_t = std::vector<edge_t*>;
    using path_view_t = std::span<edge_t* const>;
    using stack_entry_t = decltype(std::ranges::subrange{std::declval<edges_view_t>()});

    std::vector<stack_entry_t> stack{};
    std::vector<State> states{};
    path_t path{};

    stack.emplace_back(edges_view_of_fn(root));
    states.push_back(initial_state);

    while(!stop_fn()) {
        if(stack.back().empty()) {
            stack.pop_back();
            if(stack.empty())
                return true;

            auto size = states.size();
            backtrack_fn(states[size - 2], states[size - 1], path_view_t{path.data(), size - 1});
            states.pop_back();
        }
        else {
            auto* nextEdge = &stack.back().front();
            if(path.size() < stack.size())
                path.emplace_back(nextEdge);
            path[stack.size() - 1] = nextEdge;


            if(auto stateOpt = next_state_fn(states.back(), path_view_t{path.data(), stack.size()})) {
                // not at dfs depth, add next node
                stack.emplace_back(edges_view_of_fn(destination_of_fn(stack.back().front())));
                states.emplace_back(std::move(*stateOpt));
                continue; // do not advance new node
            }
        }
        stack.back().advance(1);
    }

    return false;
}

}


namespace lbs {
template<std::unsigned_integral T>
[[nodiscard]] constexpr T delete_bit(T x, size_t idx) {
    auto const mask = static_cast<T>((T{1} << idx) - T{1});

    T const lower = x & mask;
    T const upper = (x >> 1) & ~mask;
    return lower | upper;
}
}
