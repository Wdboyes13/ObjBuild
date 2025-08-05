#!/bin/bash

if [ "$(uname)" = "Linux" ]; then 
    echo "Detected Linux"
    export CC=gcc
    export CXX=g++
    g++ -o build BUILD.cpp  
fi

if [ "$(uname)" = "Darwin" ]; then 
    echo "Detected macOS"
    export CC=clang
    export CXX=clang++
    clang++ -o build BUILD.cpp  
fi