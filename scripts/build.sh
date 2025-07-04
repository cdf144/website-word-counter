#!/bin/bash

cmake -B build -S . -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1
cmake --build build
