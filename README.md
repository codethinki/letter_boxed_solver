# letter_boxed_solver
## introduction
solves the game

## usage
1. start
1. enter sides 1 by 1
1. start

## mechanism
1. simd friendly wordlist separator parsing (auto vectorized by clang). Not needed if using constexpr wordlist (enabled per default)
1. filter word list for allowed characters & character sequences with overlapping bitmasks (no overlap => valid)
1. construct graph with words as edges and their character mask connecting allowed character nodes
1. traverse graph with iteratively deepening dfs and visited cache pruning longer than optimal paths (after threshold)


## compiling yourself

### what to know
- contains settings.hpp with some constants e.g. max solution count

### requirements
- c++ 23 with #embed support
- cth library "install" path specified in the CMakePresets.json (default: ´${ProjectDir}../cth/out/install/`



## Other stuff
Algorithms are theoretically fully constexpr (if you want to solve this problem at compile time for some reason)

**Sources: the wordlists are copied from [github/aliceyliang](https://github.com/aliceyliang/letter-boxed-solver) **

Has a chatgpt oneshot solution included as comparison.