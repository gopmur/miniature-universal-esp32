cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(http_server)

# set(HTTP_ASSETS_GENERATOR python ${CMAKE_SOURCE_DIR}/main/scripts/generate_http_assets.py)
# set(GENERATED_DIR ${CMAKE_SOURCE_DIR}/build/generated)
# set(GENERATED_HTTP_ASSETS_HEADER ${GENERATED_DIR}/inc/http_assets.h)
# set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/main/assets)

# file(GLOB_RECURSE ASSET_FILES "${ASSETS_DIR}/*")

# add_custom_command(
#   OUTPUT "${GENERATED_HTTP_ASSETS_HEADER}"
#   COMMAND ${HTTP_ASSETS_GENERATOR} ${ASSETS_DIR} ${GENERATED_DIR}
#   DEPENDS ${ASSET_FILES}
# )

# add_custom_target(generate_http_assets DEPENDS ${GENERATED_HTTP_ASSETS_HEADER})

# add_dependencies(app generate_http_assets)