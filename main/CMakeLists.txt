file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

idf_component_register(
  SRCS 
    ${SOURCE_FILES}
  EMBED_TXTFILES 
    "assets/index.html" 
    "assets/manifest.webmanifest" 
    "assets/registerSW.js" 
    "assets/sw.js" 
    "assets/workbox-5ffe50d4.js"
  # EMBED_FILES 
  #   "assets/icon-192.png"
  #   "assets/icon-512.png"
  INCLUDE_DIRS 
    "inc"
)
