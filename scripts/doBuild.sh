#!/usr/bin/env bash

mkdir -p build

cd build

cmake --S .. -B .

cmake --build . -j$(nproc) --config=Release
