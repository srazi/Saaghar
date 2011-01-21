TEMPLATE = app
TARGET = Saaghar

DEPENDPATH += . build
INCLUDEPATH += . $UI_DIR

CONFIG	+= qt warn_off release

#for static building un-comment the following two lines
#CONFIG	+= static
#DEFINES += STATIC

QT += sql

# Input
HEADERS += mainwindow.h \
	SearchItemDelegate.h \
	SaagharWidget.h \
	QGanjoorDbBrowser.h \
	QGanjoorDbStuff.h \
	settings.h \
	version.h
FORMS += mainwindow.ui \
	settings.ui
SOURCES += main.cpp \
	mainwindow.cpp \
	SearchItemDelegate.cpp \
    SaagharWidget.cpp \
    QGanjoorDbBrowser.cpp \
	settings.cpp
	
RESOURCES += saaghar.qrc

TRANSLATIONS += saaghar_fa.ts

message("'make install' doesn't overwrite existing 'database file', do that manually!")

win32 {

win32-msvc*{
    RC_FILE = win.rc
}

QTPLUGIN += qsqlite

target.path = Saaghar-Win
INSTALLS = target
utilities.path = Saaghar-Win
utilities.files = utilities/AUTHORS \
	utilities/CHANGELOG \
	utilities/COPYING \
	utilities/README \
	utilities/TODO \
	saaghar.ico

poetsImage.path = Saaghar-Win/poets_images
poetsImage.files = utilities/poets_images/10.png \
	utilities/poets_images/11.png \ 
	utilities/poets_images/12.png \ 
	utilities/poets_images/13.png \ 
	utilities/poets_images/14.png \ 
	utilities/poets_images/15.png \ 
	utilities/poets_images/16.png \ 
	utilities/poets_images/17.png \ 
	utilities/poets_images/18.png \ 
	utilities/poets_images/19.png \ 
	utilities/poets_images/2.png \ 
	utilities/poets_images/20.png \ 
	utilities/poets_images/21.png \ 
	utilities/poets_images/22.png \ 
	utilities/poets_images/23.png \ 
	utilities/poets_images/24.png \ 
	utilities/poets_images/25.png \ 
	utilities/poets_images/26.png \ 
	utilities/poets_images/27.png \ 
	utilities/poets_images/28.png \ 
	utilities/poets_images/29.png \ 
	utilities/poets_images/3.png \ 
	utilities/poets_images/30.png \ 
	utilities/poets_images/31.png \ 
	utilities/poets_images/32.png \ 
	utilities/poets_images/33.png \ 
	utilities/poets_images/34.png \ 
	utilities/poets_images/35.png \ 
	utilities/poets_images/37.png \ 
	utilities/poets_images/38.png \ 
	utilities/poets_images/39.png \ 
	utilities/poets_images/4.png \ 
	utilities/poets_images/40.png \ 
	utilities/poets_images/41.png \ 
	utilities/poets_images/42.png \ 
	utilities/poets_images/43.png \ 
	utilities/poets_images/44.png \ 
	utilities/poets_images/45.png \ 
	utilities/poets_images/46.png \ 
	utilities/poets_images/47.png \ 
	utilities/poets_images/5.png \ 
	utilities/poets_images/501.png \ 
	utilities/poets_images/502.png \ 
	utilities/poets_images/503.png \ 
	utilities/poets_images/504.png \ 
	utilities/poets_images/505.png \ 
	utilities/poets_images/6.png \ 
	utilities/poets_images/600.png \ 
	utilities/poets_images/602.png \ 
	utilities/poets_images/7.png \ 
	utilities/poets_images/8.png \ 
	utilities/poets_images/9.png

INSTALLS += utilities \
	poetsImage
}

mac {
LOG = $$system(mkdir ~/Library/)
LOG += $$system(mkdir ~/Library/Saaghar)
LOG  += $$system(cp -n utilities/* ~/Library/Saaghar/)

CONFIG += link_prl x86
QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.5.sdk
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5

target.path = /Applications
INSTALLS = target
utilities.path = Contents/Resources
utilities.files = utilities/AUTHORS \
	utilities/CHANGELOG \
	utilities/COPYING \
	utilities/README \
	utilities/TODO

poetsImage.path = Contents/Resources/poets_images
poetsImage.files = utilities/poets_images/10.png \
	utilities/poets_images/11.png \ 
	utilities/poets_images/12.png \ 
	utilities/poets_images/13.png \ 
	utilities/poets_images/14.png \ 
	utilities/poets_images/15.png \ 
	utilities/poets_images/16.png \ 
	utilities/poets_images/17.png \ 
	utilities/poets_images/18.png \ 
	utilities/poets_images/19.png \ 
	utilities/poets_images/2.png \ 
	utilities/poets_images/20.png \ 
	utilities/poets_images/21.png \ 
	utilities/poets_images/22.png \ 
	utilities/poets_images/23.png \ 
	utilities/poets_images/24.png \ 
	utilities/poets_images/25.png \ 
	utilities/poets_images/26.png \ 
	utilities/poets_images/27.png \ 
	utilities/poets_images/28.png \ 
	utilities/poets_images/29.png \ 
	utilities/poets_images/3.png \ 
	utilities/poets_images/30.png \ 
	utilities/poets_images/31.png \ 
	utilities/poets_images/32.png \ 
	utilities/poets_images/33.png \ 
	utilities/poets_images/34.png \ 
	utilities/poets_images/35.png \ 
	utilities/poets_images/37.png \ 
	utilities/poets_images/38.png \ 
	utilities/poets_images/39.png \ 
	utilities/poets_images/4.png \ 
	utilities/poets_images/40.png \ 
	utilities/poets_images/41.png \ 
	utilities/poets_images/42.png \ 
	utilities/poets_images/43.png \ 
	utilities/poets_images/44.png \ 
	utilities/poets_images/45.png \ 
	utilities/poets_images/46.png \ 
	utilities/poets_images/47.png \ 
	utilities/poets_images/5.png \ 
	utilities/poets_images/501.png \ 
	utilities/poets_images/502.png \ 
	utilities/poets_images/503.png \ 
	utilities/poets_images/504.png \ 
	utilities/poets_images/505.png \ 
	utilities/poets_images/6.png \ 
	utilities/poets_images/600.png \ 
	utilities/poets_images/602.png \ 
	utilities/poets_images/7.png \ 
	utilities/poets_images/8.png \ 
	utilities/poets_images/9.png
 
QMAKE_BUNDLE_DATA += utilities\
                     poetsImage
ICON = saaghar.icns
QMAKE_INFO_PLIST = Info.plist

message("'make uninstall' doesn't remove "$$(HOME)/Library/Saaghar" directory and its contents especially 'database file', do that manually!")
}

unix:!macx {
TARGET = saaghar
LOG = $$system(mkdir ~/.Pojh)
LOG += $$system(mkdir ~/.Pojh/Saaghar)
LOG  += $$system(cp -n utilities/* ~/.Pojh/Saaghar/)

PREFIX = /usr
DESKTOPDIR = $${PREFIX}/share/applications
ICONDIR = $${PREFIX}/share/pixmaps

target.path = $${PREFIX}/bin
INSTALLS = target

utilities.path = $${PREFIX}/share/saaghar
utilities.files = utilities/AUTHORS \
	utilities/CHANGELOG \
	utilities/COPYING \
	utilities/README \
	utilities/TODO

poetsImage.path = $${PREFIX}/share/saaghar/poets_images
poetsImage.files = utilities/poets_images/10.png \
	utilities/poets_images/11.png \ 
	utilities/poets_images/12.png \ 
	utilities/poets_images/13.png \ 
	utilities/poets_images/14.png \ 
	utilities/poets_images/15.png \ 
	utilities/poets_images/16.png \ 
	utilities/poets_images/17.png \ 
	utilities/poets_images/18.png \ 
	utilities/poets_images/19.png \ 
	utilities/poets_images/2.png \ 
	utilities/poets_images/20.png \ 
	utilities/poets_images/21.png \ 
	utilities/poets_images/22.png \ 
	utilities/poets_images/23.png \ 
	utilities/poets_images/24.png \ 
	utilities/poets_images/25.png \ 
	utilities/poets_images/26.png \ 
	utilities/poets_images/27.png \ 
	utilities/poets_images/28.png \ 
	utilities/poets_images/29.png \ 
	utilities/poets_images/3.png \ 
	utilities/poets_images/30.png \ 
	utilities/poets_images/31.png \ 
	utilities/poets_images/32.png \ 
	utilities/poets_images/33.png \ 
	utilities/poets_images/34.png \ 
	utilities/poets_images/35.png \ 
	utilities/poets_images/37.png \ 
	utilities/poets_images/38.png \ 
	utilities/poets_images/39.png \ 
	utilities/poets_images/4.png \ 
	utilities/poets_images/40.png \ 
	utilities/poets_images/41.png \ 
	utilities/poets_images/42.png \ 
	utilities/poets_images/43.png \ 
	utilities/poets_images/44.png \ 
	utilities/poets_images/45.png \ 
	utilities/poets_images/46.png \ 
	utilities/poets_images/47.png \ 
	utilities/poets_images/5.png \ 
	utilities/poets_images/501.png \ 
	utilities/poets_images/502.png \ 
	utilities/poets_images/503.png \ 
	utilities/poets_images/504.png \ 
	utilities/poets_images/505.png \ 
	utilities/poets_images/6.png \ 
	utilities/poets_images/600.png \ 
	utilities/poets_images/602.png \ 
	utilities/poets_images/7.png \ 
	utilities/poets_images/8.png \ 
	utilities/poets_images/9.png

desktop.path = $${DESKTOPDIR}
desktop.files = utilities/Saaghar.desktop
INSTALLS += desktop \
            poetsImage \
            utilities

icon.path = $${ICONDIR}
icon.files = images/saaghar.png
INSTALLS += icon

message("'make uninstall' doesn't remove "$$(HOME)/.Pojh/Saaghar" directory and its contents especially 'database file', do that manually!")
}
