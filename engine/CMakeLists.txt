set(ENGINE_SRCS
    gfx/font.cpp
    gfx/font.h
    gfx/gfx.cpp
    gfx/gfx.h
    gfx/spritesheet.cpp
    gfx/spritesheet.h

    input/input.cpp
    input/input.h

    os/file.cpp
    os/file.h
    os/os.cpp
    os/os.h
    os/window.cpp

    sound/sound.cpp
    sound/sound.h

    engine.cpp
    engine.h
)

add_library(engine
    ${ENGINE_SRCS}
)
target_link_libraries(engine zlib libpng stb cjson)

macro(add_game GAME SRCS)
    message(STATUS "Adding game: ${GAME}")
    add_executable(${GAME} ${SRCS})
    target_link_libraries(${GAME} engine)
    set_target_properties(${GAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    set_target_properties(${GAME} PROPERTIES FOLDER games)
endmacro()
