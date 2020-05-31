# clox

Yet another implementation of the Lox language in C.

- http://www.craftinginterpreters.com
- https://github.com/munificent/craftinginterpreters

## Overview

This is a tree-walk interpreter for the Lox language. It has all languages
features and passes all checks, except:

- [for: closure in body](https://github.com/munificent/craftinginterpreters/blob/master/test/for/closure_in_body.lox)
- [function: local mutual recursion](https://github.com/munificent/craftinginterpreters/blob/master/test/function/local_mutual_recursion.lox)
- [collide_with_parameter.lox](https://github.com/munificent/craftinginterpreters/blob/master/test/variable/collide_with_parameter.lox)

For now there is no Garbage Collector for objects, so some programs can consume
a lot of memory 
