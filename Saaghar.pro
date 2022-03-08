TEMPLATE = app
TARGET = Saaghar

CONFIG += qt


CONFIG(debug, debug|release) {
    !build_pass:message("DEBUG BUILD")
    DEFINES += SAAGHAR_DEBUG
} else {
    !build_pass:message("RELEASE BUILD")
##un-comment the following two lines for skipping all warning and debug messages.
#    DEFINES += QT_NO_WARNING_OUTPUT
#    DEFINES += QT_NO_DEBUG_OUTPUT
}

##un-comment for static build
#CONFIG += static

##un-comment for support phonon on Qt5, it needs phonon4qt5
#CONFIG += USE_PHONON4_QT5

CONFIG(static) {
    !build_pass:message("STATIC BUILD")
    DEFINES += STATIC
}

# we have to bump up this over time, so it help us to update application in a continuous manner.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00


greaterThan(QT_MAJOR_VERSION, 4) {
    QT += network widgets printsupport sql xml concurrent

    DEFINES += MEDIA_PLAYER
    isEqual(QT_MAJOR_VERSION, 6) {
        QT += core5compat
    }
} else {
    DEFINES += MEDIA_PLAYER

    QT += sql network xml
}


!win32 {
# Git revision
rev = $$system(sh $$PWD/scripts/getrevision.sh)

isEmpty(rev) {
    rev = ""
}

!build_pass:message("Git revision: " $$rev)
}

DEFINES += GIT_REVISION=\\\"""$$rev"\\\""

win32 {
    target.path = Saaghar-Win
    RESOURCES_PATH = Saaghar-Win

!static {
    ##shared libs
    depFiles.path = Saaghar-Win
    depFiles.files = $$[QT_INSTALL_BINS]/zlib1.dll \
        $$[QT_INSTALL_BINS]/libssl32.dll \
        $$[QT_INSTALL_BINS]/ssleay32.dll

    phononBackend.path = Saaghar-Win/phonon_backend
    phononBackend.files = $$[QT_INSTALL_PLUGINS]/phonon_backend/phonon_ds94.dll

    sqlPlugins.path = Saaghar-Win/sqldrivers
    imagePlugins.path = Saaghar-Win/imageformats

isEqual(QT_MAJOR_VERSION, 5) {
    sqlPlugins.files = $$[QT_INSTALL_PLUGINS]/sqldrivers/qsqlite.dll

    imagePlugins.files = $$[QT_INSTALL_PLUGINS]/imageformats/qgif.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qico.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qjpeg.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qsvg.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qtiff.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qdds.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qicns.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qtga.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qwbmp.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qwebp.dll \

    depFiles.files += $$[QT_INSTALL_BINS]/Qt5Core.dll \
        $$[QT_INSTALL_BINS]/Qt5Gui.dll \
        $$[QT_INSTALL_BINS]/Qt5Sql.dll \
        $$[QT_INSTALL_BINS]/Qt5Network.dll \
        $$[QT_INSTALL_BINS]/Qt5Xml.dll \
        $$[QT_INSTALL_BINS]/Qt5Widgets.dll \
        $$[QT_INSTALL_BINS]/Qt5PrintSupport.dll \
        $$[QT_INSTALL_BINS]/Qt5Multimedia.dll \
        $$[QT_INSTALL_BINS]/Qt5Svg.dll

    platformPlugins.path = Saaghar-Win/platforms
    platformPlugins.files = $$[QT_INSTALL_PLUGINS]/platforms/qwindows.dll

    mediaservicePlugins.path = Saaghar-Win/mediaservice
    mediaservicePlugins.files = $$[QT_INSTALL_PLUGINS]/mediaservice/dsengine.dll

    bearerPlugins.path = Saaghar-Win/bearer
    bearerPlugins.files = $$[QT_INSTALL_PLUGINS]/bearer/qgenericbearer.dll \
        $$[QT_INSTALL_PLUGINS]/bearer/qnativewifibearer.dll

    INSTALLS += platformPlugins \
        mediaservicePlugins \
        bearerPlugins
} else {
    sqlPlugins.files = $$[QT_INSTALL_PLUGINS]/sqldrivers/qsqlite4.dll

    imagePlugins.files = $$[QT_INSTALL_PLUGINS]/imageformats/qgif4.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qico4.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qjpeg4.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qmng4.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qsvg4.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qtiff4.dll

    depFiles.files += $$[QT_INSTALL_BINS]/QtCore4.dll \
        $$[QT_INSTALL_BINS]/QtGui4.dll \
        $$[QT_INSTALL_BINS]/QtSql4.dll \
        $$[QT_INSTALL_BINS]/QtNetwork4.dll \
        $$[QT_INSTALL_BINS]/QtXml4.dll \
        $$[QT_INSTALL_BINS]/phonon4.dll
}

win32-msvc*{
    DEFINES += D_MSVC_CC
    #QTPLUGIN += qsqlite

    depFiles.files += $$[QT_INSTALL_BINS]/msvcp100.dll \
        $$[QT_INSTALL_BINS]/msvcr100.dll
}

win32-g++{
    DEFINES += D_MINGW_CC

    depFiles.files += \
        # $$[QT_INSTALL_BINS]/mingwm10.dll \
        $$[QT_INSTALL_BINS]/libgcc_s_dw2-1.dll \
        $$[QT_INSTALL_BINS]/libstdc++-6.dll \
        $$[QT_INSTALL_BINS]/libwinpthread-1.dll
}

    INSTALLS += depFiles \
        sqlPlugins \
        imagePlugins \
        phononBackend
} # end !static

    LIBS += -lzlibstat
}
else {
    LIBS += -L/usr/lib -lz
}

mac {
## Should be removed?
#    CONFIG += link_prl x86
#    QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.5.sdk
#    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5

    target.path = /Applications
    RESOURCES_PATH = Contents/Resources
}



unix:!macx {
    TARGET = saaghar

isEmpty( PREFIX ) {
    PREFIX = /usr
}

DEFINES += PREFIX=\\\"$${PREFIX}\\\"

    DESKTOPDIR = $${PREFIX}/share/applications
    ICONDIR = $${PREFIX}/share/pixmaps

    target.path = $${PREFIX}/bin
    RESOURCES_PATH = $${PREFIX}/share/saaghar
    desktop.path = $${DESKTOPDIR}
    icon.path = $${ICONDIR}

    INSTALLS += desktop icon
}

utilities.path = $${RESOURCES_PATH}
utilities.files += \
    $$PWD/AUTHORS \
    $$PWD/CHANGELOG \
    $$PWD/GPLv3 \
    $$PWD/GPLv2Fa \
    $$PWD/README.md \
    $$PWD/TODO \
    $$PWD/LICENSE

include($$PWD/src/src.pri)
include($$PWD/locale/locale.pri)
include($$PWD/data/data.pri)
include($$PWD/doc/doc.pri)

INSTALLS += target

mac {
    QMAKE_BUNDLE_DATA += utilities \
        backgrounds \
        classicIcons \
        lightGrayIcons \
        iconicCyanIcons
}
else {
    INSTALLS += utilities \
        backgrounds \
        classicIcons \
        lightGrayIcons \
        iconicCyanIcons
}

!build_pass:message("'make install' doesn't overwrite existing 'database file', do that manually!")
