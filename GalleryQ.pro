QT += qml quick svg sql \
    widgets

SOURCES += \
        src/main.cpp \
        src/model.cpp \
        src/mpv.cpp \
        src/sql.cpp \
        src/storage.cpp \
        src/thumbnail.cpp

RESOURCES += qml/qml.qrc
QML_IMPORT_PATH = qml
QML_DESIGNER_IMPORT_PATH = qml

CONFIG += qmltypes
QML_IMPORT_NAME = gallery.q
QML_IMPORT_MAJOR_VERSION = 1

INCLUDEPATH += src

win32: LIBS += -L$$PWD/windows/mpv -llibmpv.dll
win32: INCLUDEPATH += $$PWD/windows
win32: INSTALL += $$PWD/windows/mpv/llibmpv-2.dll
unix: LIBS += -lmpv
unix: INCLUDEPATH += /usr/include

HEADERS += \
    src/model.h \
    src/mpv.h \
    src/mpv_qthelper.h \
    src/sql.h \
    src/storage.h \
    src/thumbnail.h \
    src/util.h
