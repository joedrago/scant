cmake_minimum_required(VERSION 3.8)
project(scant)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4996")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(ext)

include_directories(
    ext/zlib
    ext/libpng
    ext/stb
    ext/tinyheaders
    engine
)

add_subdirectory(engine)
add_subdirectory(games)
