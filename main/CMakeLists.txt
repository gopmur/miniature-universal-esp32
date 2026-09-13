set(API_VERSION "0.0.1")
set(CORE_VERSION "0.0.1")

configure_file(
  "../main/templates/version.h.in"
  "../main/inc/version.hpp"
)

file(GLOB_RECURSE SOURCE_CPP_FILES "src/*.cpp")
file(GLOB_RECURSE SOURCE_C_FILES "src/*.c")
file(GLOB_RECURSE ASSET_FILES
  "${CMAKE_SOURCE_DIR}/generated/assets/*.gz"
  "${CMAKE_SOURCE_DIR}/main/certs/*.pem"
)

message("C Source files:")
foreach(file ${SOURCE_C_FILES})
  message("  ${file}")
endforeach()
message("CPP Source files:")
foreach(file ${SOURCE_CPP_FILES})
  message("  ${file}")
endforeach()

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
