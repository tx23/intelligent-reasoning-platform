#!/bin/sh

if [ -d "build" ]; then
    echo "remove build dir"
    rm -rf build
fi

echo "create build dir"
mkdir build

cd build
cmake ..
make

cd ../
rm -rf build

