#! /bin/bash
set -e

./build.sh &&
idf.py flash &&
idf.py monitor