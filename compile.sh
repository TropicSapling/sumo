#!/usr/bin/bash
cmake -B build .
cmake --build build -j$(nproc)

