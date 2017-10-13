cmake_minimum_required(VERSION 3.8)
project(scant)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(ext)

include_directories(
    ext/zlib
    ext/libpng
    engine
)

add_subdirectory(engine)
add_subdirectory(games)
