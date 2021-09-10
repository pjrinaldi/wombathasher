QT += core concurrent
QT -= gui network
CONFIG += debug_and_release debug_and_release_target qt build_all console
INCLUDEPATH += /usr/local/lib/
INCLUDEPATH += /usr/local/include/
VPATH += /usr/local/lib/
VPATH += /usr/local/include/
HEADERS = blake3.h
SOURCES = wombathasher.cpp
release: DESTDIR = release
debug: DESTDIR = debug
LIBS += -lblake3 -llmdb #-llz4
if(!debug_and_release|build_pass):CONFIG(debug, debug|release) {
}
target.path = /usr/local/bin
INSTALLS += target
