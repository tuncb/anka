# Containers

## arrays

A list of values that are of same type. Arrays could also hold other arrays. sub-arrays could be of different lengths.

(1 2 3 4 5)
((1 2 ) (3 4 5) (6 7 8 9))

## tuples

[1 2 3 4] Tuples are lists that can contain elements of different types. They can also contain other tuples.

[1 2 [3 4] (1 2 3 4)]

## words
Words are space separated definitions.

min filter[odd] (1 2 3)

Three words are defined here.

## sentences
A sentence is a series of words separated with line endings.
They are executed separate from each other.

min filter[odd] (1 2 3)
min filter[even] (1 2 3)

Two sentences are defined here. Execution pipeline will execute them in the order that they are defined.
## blocks
Blocks contain a series of words and sentences that should be executed together. Sentences are still executed separately but the execution context is separated from outside.
{
  foo: (1 2 3)
  min foo
}

foo #error => foo is not visible here.
When the block is executed and the last value is returned as the result of the block.



