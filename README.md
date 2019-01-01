# Advent of Code 2018

This repository contains C++17 based solutions to Advent of Code. If you intend to do the challenges yourself, I suggest that you avoid reading this code base.

All solutions are standard C++ and should compile on any conformant toolchain, however, I have have built and run these using Clang7 on Linux.

In the interests of simplicity, all solutions are in a single translation unit and specific to the part of that particular day although typically differences are tiny as most of the code is essentially a copy of part1 since the tasks so far are more of an evolution from their initial part than an entirely separate task.

Performance hasn't been a particular objective but the solutions perform in a fairly fast manner (except Day 15 which is a bit of a mess).

## Compiling
No build system has been used, but since these are a single translation unit each, something along these lines should do the job:
`g++ -o part1 -std=c++17 -O3 part1.cpp`
`clang++ -o part1 -std=c++17 -O3 part1.cpp`

## Usage

By default, each solution expects its input data in input.txt you can override this by passing a file path when executing the application from the command line.

## Caveats

Currently, Day 15 solutions produce an *incorrect* answer if compiled against libstdc++ - Ensure that you compile with clang and use `-stdlib=libc++` - I'll probably get around to hunting down what the heck is going on with this at some point.
