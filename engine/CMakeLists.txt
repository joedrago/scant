set(ENGINE_SRCS
    gfx/gfx.cpp
    gfx/gfx.h

    input/input.cpp
    input/input.h

    os/file.cpp
    os/file.h
    os/main.cpp
    os/os.cpp
    os/os.h

    sound/sound.cpp
    sound/sound.h

    engine.cpp
    engine.h
)

add_library(engine
    ${ENGINE_SRCS}
)

macro(add_game GAME SRCS)
    message(status "Adding game ${GAME}...")
    add_executable(${GAME} ${SRCS})
    target_link_libraries(${GAME} engine)
    set_target_properties(${GAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endmacro()
