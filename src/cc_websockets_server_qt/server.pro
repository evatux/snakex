QT = websockets

TARGET = server

CONFIG += c++17

INCLUDEPATH += ..
LIBS += -L../../build/src/cc_core -Wl,-rpath=../../build/src/cc_core \
        -lsnakexcore

SOURCES += \
    main.cpp \
    server.cpp \
    game_server.cpp \

HEADERS += \
    server.h \
    game_server.h \

