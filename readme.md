# anka

anka is an array programming language, heavely inspired by APL, q and haskell.

## Current features
* Interpreter that can load a file.
* REPL.
* data types: integer number, array of integer number
* internal functions: ioata, inc, dec, neg, abs, length
* basic pipeline support

## What you can do now
```
ioata 5
>>(1 2 3 4 5)
inc 5
>>6
ioata inc inc dec 10
>>(1 2 3 4 5 6 7 8 9 10 11)
neg dec dec inc (10 11 12 13)
>> (-9 -10 -11 -12)
```

## What is planned
```
neg mul [_1 _1] (30 20 10)
>> (-900 -400 -100)
avg: div [foldl[add 0] length]
avg (1 2 3 4 5)
>> 3
foldl[add 0] filter[{unequal[0] mod[2]}] (1 2 3 4 5)
>> 9

# calculate function from https://www.youtube.com/watch?v=wGCWlI4A5z4&t=372s
calculate[bottom top]: {
  even: equals[0] mod[2]
  foldl[add 0] filter[even] (bottom..=top)
}

# some possible implementations for internal functions
filter[predicate]: |predicate: _1|
if[trueFunc falseFunc]: |true: trueFunc false: falseFunc|
count[predicate x]: {
  nr: 0
  nr |predicate: {inc= nr}| x
}

# final value of variable after performing operations
operate[x]: {
  val: 0
  val |"--X": {dec= val} "X--": {dec= val} "++X": {inc= val} "X++": {inc= val}| x
}

#hash tables
println |(1: "one" 2: "two" 3: "three" 4: "four" 5: "five")| (1 2 3 4 5)
>> one
two
three
four
five

#Advent of Code 2022 in APL & BQN Day 1!
# array of arrays
data: ((1000 2000 3000) (4000) (5000 6000) (7000 8000 9000) (10000))
max foldl[add 0] data
# top three
top[3] sort foldl[add 0] data

# Algorithms as a Tool of Thought
equals_to_first_element[x]: foldl[and true] equals[{head x}] x
equals_to_first_element[x]: all equals[{head x}] x
equals_to_first_element: all equals [head identity]
equals_to_first_element: true |false: return[false]|
equals_to_first_element: equals[1] length unique
equals_to_first_element: all equals [...] slides[2]
```
## TODO

See .\examples\arrayExamples.anka for ideas for the fully functional future version of the language.

### Major features

* atomic type None for optional support
* Tuples
* function support: (int, int) -> int
* atomic type double
* atomic type bool
* atomic type array of double
* atomic type array of bool
* internal function versions for internal types bool and double
* user defined variables
* user defined functions
* import anka files to current context