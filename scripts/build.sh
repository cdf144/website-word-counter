#!/bin/bash

cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build
