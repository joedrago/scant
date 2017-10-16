set(PUTDUT_SRCS
    main.cpp

    App.cpp
    App.h

    Game.cpp
    Game.h

    Level.cpp
    Level.h
)

add_game(putdut
    ${PUTDUT_SRCS}
)
