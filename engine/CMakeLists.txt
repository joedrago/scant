set(ENGINE_SRCS
    gfx/gfx.cpp
    gfx/gfx.h

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

macro(add_game GAME SRCS)
    message(STATUS "Adding game: ${GAME}")
    add_executable(${GAME} ${SRCS})
    target_link_libraries(${GAME} engine)
    set_target_properties(${GAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endmacro()
