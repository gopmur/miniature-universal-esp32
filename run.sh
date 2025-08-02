#! /bin/bash

./build.sh &&
idf.py flash &&
idf.py monitor