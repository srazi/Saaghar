TEMPLATE = app
TARGET = Saaghar

CONFIG += qt
#warn_off

##un-comment the following two lines for skipping all warning and debug messages.
CONFIG(debug, debug|release) {
    !build_pass:message("DEBUG BUILD")
} else {
    !build_pass:message("RELEASE BUILD")
##un-comment for release version
#    DEFINES += QT_NO_WARNING_OUTPUT
#    DEFINES += QT_NO_DEBUG_OUTPUT
}

##for static building un-comment the following two lines
#CONFIG += static

CONFIG(static) {
    !build_pass:message("STATIC BUILD")
    DEFINES += STATIC
    DEFINES += NO_PHONON_LIB
}

QT += sql network xml

win32 {

win32-msvc*{
    DEFINES += D_MSVC_CC
    #QTPLUGIN += qsqlite
}

win32-g++{
    DEFINES += D_MINGW_CC
}
    target.path = Saaghar-Win
    RESOURCES_PATH = Saaghar-Win

!static {
    ##shared libs
    depFiles.path = Saaghar-Win
    depFiles.files = $$[QT_INSTALL_LIBS]/QtCore4.dll \
        $$[QT_INSTALL_LIBS]/QtGui4.dll \
        $$[QT_INSTALL_LIBS]/QtSql4.dll \
        $$[QT_INSTALL_LIBS]/QtNetwork4.dll \
        $$[QT_INSTALL_LIBS]/QtXml4.dll \
        $$[QT_INSTALL_BINS]/mingwm10.dll \
        $$[QT_INSTALL_BINS]/libgcc_s_dw2-1.dll

    sqlPlugins.path = Saaghar-Win/sqldrivers
    sqlPlugins.files = $$[QT_INSTALL_PLUGINS]/sqldrivers/qsqlite4.dll

    imagePlugins.path = Saaghar-Win/imageformats
    imagePlugins.files = $$[QT_INSTALL_PLUGINS]/imageformats/qgif4.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qico4.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qjpeg4.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qmng4.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qsvg4.dll \
        $$[QT_INSTALL_PLUGINS]/imageformats/qtiff4.dll

    INSTALLS += depFiles \
        sqlPlugins \
        imagePlugins
}
}

mac {
    CONFIG += link_prl x86
    QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.5.sdk
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5

    target.path = /Applications
    RESOURCES_PATH = Contents/Resources
}



unix:!macx {
    TARGET = saaghar

isEmpty( PREFIX ) {
    PREFIX = /usr
}

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
    $$PWD/GPLv2Fa-GPLv3En \
    $$PWD/README \
    $$PWD/TODO \
    $$PWD/LICENSE

include($$PWD/src/src.pri)
include($$PWD/locale/locale.pri)
include($$PWD/data/data.pri)
include($$PWD/doc/doc.pri)

INSTALLS += target

mac {
    QMAKE_BUNDLE_DATA += utilities \
        poetsImage \
        backgrounds \
        lightGrayIcons
}
else {
    INSTALLS += utilities \
        poetsImage \
        backgrounds \
        lightGrayIcons
}

!build_pass:message("'make install' doesn't overwrite existing 'database file', do that manually!")
