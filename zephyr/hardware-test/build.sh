#!/bin/sh

echo `pwd`/../boards

mkdir -p build && cd build && \
cmake -v -GNinja -DBOARD_ROOT=`pwd`/../boards -DBOARD=badge -DBOARD_DIR=../../boards/arm/badge .. && \
ninja
