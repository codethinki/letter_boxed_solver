#pragma once
#include "algorithm.hpp"
#include "index.hpp"
#include "util.hpp"

#include <array>
#include <functional>
#include <ranges>
#include <string_view>
#include <vector>


namespace lbs {
struct node;

struct word_edge {
    char_mask_t wordHash;
    node const* to;

    std::vector<size_t> wordIndices{};
};

struct node {
    std::vector<word_edge> outgoing;
    size_t compressedIdx;
    size_t incoming;
};


namespace dev {
    using nodes_view_t = std::span<node const, BOX_CHARS>;

    [[nodiscard]] constexpr node router_node(nodes_view_t nodes) {
        node router{};
        router.outgoing.reserve(nodes.size());
        for(size_t i = 0; i < nodes.size(); i++)
            router.outgoing.emplace_back(char_mask_t{}, &nodes[i]);

        return router;
    }

    using lala = int;
    [[nodiscard]] constexpr bool early_terminate_instance(nodes_view_t nodes) {
        size_t onlyOutgoing = 0;
        size_t onlyIncoming = 0;

        lala a = 0;

        for(auto& node : nodes) {
            bool noIncoming = node.incoming == 0;
            bool noOutcoming = node.outgoing.empty();

            if(noIncoming && noOutcoming)
                return true;

            if(noOutcoming)
                ++onlyOutgoing;
            if(noIncoming)
                ++onlyIncoming;
        }
        return onlyOutgoing > 1 || onlyIncoming > 1;
    }

    struct state_cache_entry {
        uint8_t depthCeiling = 255;
        uint8_t creationDepth = 0;
    };

    [[nodiscard]] constexpr char_mask_t compress_hash(char_mask_t hash, size_t idx) {
        return delete_bit(hash, idx);
    }

    [[nodiscard]] constexpr std::vector<std::vector<word_edge const*>> solve_raw(
        node const& router,
        nodes_view_t nodes
    ) {

        // no solution
        if(early_terminate_instance(nodes))
            return {};


        std::vector<std::vector<word_edge const*>> solutions{};

        static constexpr size_t MAX_STATES = 1uz << (BOX_CHARS - 1);

        std::vector<state_cache_entry> stateDepthCache(MAX_STATES * BOX_CHARS);

        // +1 bc of router

        for(size_t depth = MIN_SOLUTION_SIZE + 1; depth <= MAX_SEARCH_SIZE + 1 && solutions.size() < MAX_SOLUTIONS; depth++) {
            dfs(
                router,
                [](node const& node) -> std::vector<word_edge> const& { return node.outgoing; },
                [](word_edge const& edge) -> node const& { return *edge.to; },
                char_mask_t{0},
                [&stateDepthCache, &solutions, depth](char_mask_t prev, auto&& ptr_rng) -> std::optional<char_mask_t> {
                    auto const pathSize = std::ranges::size(ptr_rng);
                    // edge from router doesn't count
                    if(pathSize <= 1)
                        return char_mask_t{0};

                    // check path
                    word_edge const* lastEdge = ptr_rng.back();
                    auto const pathHash = prev | lastEdge->wordHash;

                    // check solution
                    if(pathSize == depth) {
                        if(pathHash == solution_mask)
                            solutions.emplace_back(std::from_range, ptr_rng);

                        return std::nullopt; // prune after solution
                    }

                    if(depth <= STATE_CACHE_MIN_DEPTH)
                        return pathHash;


                    auto const nextIdx = lastEdge->to->compressedIdx;
                    size_t cacheHash = nextIdx * MAX_STATES + compress_hash(pathHash, nextIdx);


                    auto& [depthCeiling, creationDepth] = stateDepthCache[cacheHash];
                    if(depthCeiling < pathSize || (depthCeiling <= pathSize && creationDepth >= depth))
                        return std::nullopt;

                    depthCeiling = pathSize;
                    creationDepth = static_cast<uint8_t>(depth);
                    return pathHash;
                },
                default_backtrack_fn,
                [&solutions] { return solutions.size() >= MAX_SOLUTIONS; }
            );
        }


        return solutions;
    }
}



/**
 * solves the letter boxed problem
 * @param compression_index of allowed characters
 * @param words to use
 * @param word_hashes for words (character masks)
 * @return chain of words to use
 */
[[nodiscard]] constexpr std::vector<std::vector<std::vector<std::string_view>>> solve(
    compression_index_t const& compression_index,
    std::span<std::string_view const> words,
    std::span<char_mask_t const> word_hashes
) {
    CTH_CRITICAL(words.size() != word_hashes.size(), "hashes must match words") {}


    std::array<node, BOX_CHARS> nodes{};
    auto node_at = [&nodes, &compression_index](char c) {
        return &nodes[compression_index[idx(c)]];
    };
    for(size_t i = 0; i < nodes.size(); i++)
        nodes[i].compressedIdx = i;

    for(size_t i = 0; i < words.size(); i++) {
        auto const& w = words[i];
        auto* to = node_at(w.back());
        auto const& hash = word_hashes[i];

        auto& outgoing = node_at(w.front())->outgoing;

        auto foundIt = std::ranges::find_if(
            outgoing,
            [hash, to](word_edge const& e) {
                return e.wordHash == hash && e.to == to;
            }
        );

        if(foundIt == outgoing.end()) {
            outgoing.emplace_back(hash, to, std::vector{i});
            ++to->incoming;
        }
        else
            foundIt->wordIndices.emplace_back(i);
    }

    auto const routerNode = dev::router_node(nodes);
    auto rawSolutions = dev::solve_raw(routerNode, nodes);

    std::vector<std::vector<std::vector<std::string_view>>> solutions{};


    // trace solutions
    for(auto& rawSolution : rawSolutions) {
        auto& solution = solutions.emplace_back();
        solution.reserve(rawSolution.size() - 1);
        // drop 1 bc of router
        for(auto const& edge : rawSolution | std::views::drop(1)) {
            auto& indices = edge->wordIndices;
            auto& pathWords = solution.emplace_back();
            pathWords.reserve(indices.size());
            for(auto& wordIdx : indices)
                pathWords.emplace_back(words[wordIdx]);
        }
    }
    return solutions;

}
}
