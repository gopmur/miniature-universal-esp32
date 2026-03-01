#! /bin/bash
set -e

export IDF_TOOLCHAIN=clang

function pull_and_build_web_app() {
  cd app/hexa-webapp
  git pull
  bun i
  bun run build
  cd ../..
}

function build_web_app() {
  cd app/hexa-webapp
  bun i
  bun run build
  cd ../..
}

function generate_assets() {
  rm -rf ./build/generated
  python ./main/scripts/generate_http_assets.py ./app/hexa-webapp/dist ./generated
}

function build_firmware() {
  if [[ -z "$IDF_PATH" ]]; then
  source "$HOME/esp/v5.4.2/esp-idf/export.sh"
  fi
  idf.py build
}

build_web_app
generate_assets
build_firmware