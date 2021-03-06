#!/bin/bash

set -e

autoreconf -f -i

echo >&2 "Baseline"
./configure CFLAGS='-O3 -march=native' CC=gcc >/dev/null
make clean >/dev/null
make >/dev/null
./ray-headless


echo >&2 "Profiling"
./configure CFLAGS='-O3 -march=native -fprofile-generate' CC=gcc >/dev/null
make clean >/dev/null
make >/dev/null
./ray-headless

echo >&2 "Using profile"
./configure CFLAGS='-O3 -march=native -fprofile-use -fprofile-correction' CC=gcc >/dev/null
make clean >/dev/null
make >/dev/null
./ray-headless
