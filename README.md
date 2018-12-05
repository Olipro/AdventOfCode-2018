# Advent of Code 2018

This repository contains C++17 based solutions to Advent of Code. If you intend to do the challenges yourself, I suggest that you avoid reading this code base.

All solutions are standard C++ and should compile on any conformant toolchain, however, I have have built and run these using Clang7 on Linux.

In the interests of simplicity, all solutions are in a single translation unit and specific to the part of that particular day although typically differences are tiny as most of the code is essentially a copy of part1 since the tasks so far are more of an evolution from their initial part than an entirely separate task.

Performance hasn't been a particular objective - with the exception of day 3 (as I didn't flatten the grid or derive a more efficient solution) all implementations perform reasonably quickly.

# Usage

By default, each solution expects its input data in input.txt you can override this by passing a file path when executing the application from the command line.

