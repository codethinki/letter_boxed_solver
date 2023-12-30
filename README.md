# letter_boxed_solver
## introduction
nothing to explain lol

## requirements
i used:
- windows
- c++ latest
- c 2017
- llvm 17

## what to know
- the project includes a globalVariables.hpp file, where you can configure the solver
- you can input a custom wordlist but prepare it before by following the instructions in globalVariables.hpp

## how it works
1. gets the input of the letter-boxed-sides from user input
2. builds a dictionary by discarding words that either have invalid characters or characters from the same side following each other
3. loops through all possible dictionary word combinations and saves the solutions (probably very inefficient)
4. sorts the solutions by length
5. resizes the solutions to MAX_SOLUTIONS
6. prints every remaining solution with the shortest as "best"

## Issues
report all of them except cross platform or cmake stuff i haven't learned that yet so i can't help

## Other stuff
**IMPORTANT: the wordlists are copied from [github/aliceyliang](https://github.com/aliceyliang/letter-boxed-solver) (but self prepared)**
