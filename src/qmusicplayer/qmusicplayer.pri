isEqual(QT_MAJOR_VERSION, 5) {
    CONFIG(USE_PHONON4_QT5) {
        DEFINES += USE_PHONON
        QT += phonon4qt5
    } else {
        QT += multimedia
    }
} else {
    DEFINES += USE_PHONON
    QT += phonon
}

INCLUDEPATH += $$PWD

HEADERS   += \
    $$PWD/qmusicplayer.h \
    $$PWD/lyricsmanager.h
SOURCES   += \
    $$PWD/qmusicplayer.cpp \
    $$PWD/lyricsmanager.cpp

RESOURCES += $${PWD}/qmusicplayer.qrc
