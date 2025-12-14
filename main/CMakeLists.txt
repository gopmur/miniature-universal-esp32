file(GLOB_RECURSE SOURCE_CPP_FILES "src/*.cpp")
file(GLOB_RECURSE SOURCE_C_FILES "src/*.c")
file(GLOB_RECURSE ASSET_FILES "${CMAKE_SOURCE_DIR}/generated/assets/*.gz")

idf_component_register(
  SRCS
    ${SOURCE_C_FILES}
    ${SOURCE_CPP_FILES}
  EMBED_FILES
    ${ASSET_FILES}
  INCLUDE_DIRS 
    "inc"
    "${CMAKE_SOURCE_DIR}/generated/inc"
)
