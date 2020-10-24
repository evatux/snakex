QT = websockets

TARGET = client

CONFIG += c++17

INCLUDEPATH += ..
LIBS += -L../../build/src/cc_core -Wl,-rpath=../../build/src/cc_core \
        -lsnakexcore -lncurses

SOURCES += \
    main.cpp \
    client.cpp \
    render.cpp \

HEADERS += \
    client.h \
    render.h \

