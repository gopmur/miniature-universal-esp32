#! /bin/bash

IDF_TOOLCHAIN=clang
python ./main/scripts/generate_http_assets.py ./main/assets ./build/generated
if [[ -z "$IDF_PATH" ]]; then
  source "$HOME/esp/v5.4.2/esp-idf/export.sh"
fi
idf.py build