# letter_boxed_solver
pretty self explanatory how to use

## requirements
i used:
- windows
- c++ latest
- c 2017
- llvm 17

## how it works
1. gets the input of the letter-boxed-sides from user input
2. builds a dictionary by discarding words that either have invalid characters or characters from the same side following each other
3. loops through all possible dictionary word combinations and saves the solutions (probably very inefficient)
4. sorts the solutions by length
5. resizes the solutions to MAX_SOLUTIONS
6. prints every remaining solution with the shortest as "best"
