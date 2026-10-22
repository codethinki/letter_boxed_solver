[![Build & Test](https://github.com/codethinki/letter_boxed_solver/actions/workflows/build-test.yml/badge.svg)](https://github.com/codethinki/letter_boxed_solver/actions/workflows/build-test.yml)

# letter_boxed_solver
## introduction
solves the game

## usage
1. run program
1. enter sides 1 by 1
1. start

median over 558 nyt puzzles (2025-03 to 2026-10) on an i5-11300H, gcc 16 release build: `0.5ms` with the constexpr word list, `0.7ms` when loading the word list at runtime (`lbs_solver_bench`)

## mechanism
1. wordlist separator parsing with `std::simd` (64 byte blocks, separator mask → indices via `countr_zero`), without `std::simd` blockwise compaction (auto vectorized by clang). Not needed if using constexpr wordlist (enabled per default)
1. filter word list for allowed characters & character sequences with overlapping bitmasks (no overlap => valid)
1. construct graph with words as edges and their character mask connecting allowed character nodes
1. traverse graph with iteratively deepening dfs and visited cache pruning longer than optimal paths (after threshold)


## compiling yourself

### what to know
- contains settings.hpp with some constants e.g. max solution count

### requirements
- c++ 23, uses #embed when supported (otherwise the word list is loaded at runtime)
- cmake 4.1+, ninja
- [vcpkg](https://github.com/microsoft/vcpkg) with `VCPKG_ROOT` set
- cth is included as package in `lib/cth`

```bash
git clone --recursive https://github.com/codethinki/letter_boxed_solver.git
cd letter_boxed_solver
cmake --preset gcc_release
cmake --build out/build/gcc_release
```



## Other stuff
Algorithms are theoretically fully constexpr (if you want to solve this problem at compile time for some reason)

**Sources: the wordlists are copied from [github/aliceyliang](https://github.com/aliceyliang/letter-boxed-solver) **

Has a chatgpt oneshot solution included as comparison (about 2x slower at the median).
