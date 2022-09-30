# Compute Binary16 Reciprocal Using Newton's Method

## Description

Compute binary16 floating point reciprocal using Newton's Method. The code is written in C++. The code uses a self defined class to implement the IEEE 754 binary16 type.

The code was developed using Apple's Xcode version 13.4.1. It has not been tested with any other compilers.

The code is a quick and dirty hack. It IS NOT production quality code. Use at your own risk.

The code is known to produce inaccurate results in the last bit, or two of the result. It is strictly "Proof of Concept" code.

The code is explained in the video series [Building an FPU in Verilog](https://www.youtube.com/playlist?list=PLlO9sSrh8HrwcDHAtwec1ycV-m50nfUVs).
See the video *Building an FPU In Verilog: Floating Point Division, Part 3*.

## Manifest

|   Filename |                        Description                           |
|------------|--------------------------------------------------------------|
| README.md  | This file.                                                   |
| main.cpp   | Program which calculates the reciprocal of binary16 Normal numbers. |
| hp.cpp     | Methods for binary16 data type.                              |
| hp.hpp     | Header file for binary16 data type.                          |

## Copyright

:copyright: Chris Larsen, 2022
