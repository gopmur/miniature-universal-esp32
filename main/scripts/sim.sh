#! /bin/bash
set -e

export IDF_TOOLCHAIN=clang

idf.py build merge-bin
esp-emu --chip esp32c5 --firmware build/merged-binary.bin --net "user,hostfwd=tcp:127.0.0.1:8080-:80" --elf build/miniature-universal-esp32.elf --wifi-fw-ap-password 12345678
