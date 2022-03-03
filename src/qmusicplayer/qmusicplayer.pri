greaterThan(QT_MAJOR_VERSION, 4) {
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
    $$PWD/lyricsmanager.h \
    $$PWD/audiorepodownloader.h
SOURCES   += \
    $$PWD/qmusicplayer.cpp \
    $$PWD/lyricsmanager.cpp \
    $$PWD/audiorepodownloader.cpp
FORMS +=  \
    $$PWD/audiorepodownloader.ui

RESOURCES += $${PWD}/qmusicplayer.qrc
