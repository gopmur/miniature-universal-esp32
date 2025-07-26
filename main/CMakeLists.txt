file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE ASSET_FILES "${CMAKE_SOURCE_DIR}/build/generated/assets/*.gz")

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/build/generated/inc)

idf_component_register(
  SRCS 
    ${SOURCE_FILES}
  EMBED_FILES 
    ${ASSET_FILES}
  INCLUDE_DIRS 
    "inc"
    "${CMAKE_SOURCE_DIR}/build/generated/inc"
)
