TEMPLATE = app
TARGET = Saaghar

isEmpty( SOURCE_DIR ) {
SOURCE_DIR = .
}

DEPENDPATH += $${SOURCE_DIR}/ $${SOURCE_DIR}/build \
		$${SOURCE_DIR}/widgets/QMultiSelectWidget \
		$${SOURCE_DIR}/widgets/QSearchLineEdit \
		$${SOURCE_DIR}/widgets/QTextBrowserDialog

INCLUDEPATH += $${SOURCE_DIR}/ $UI_DIR $${SOURCE_DIR}/sqlite $${SOURCE_DIR}/sqlite-driver/sqlite \
		$${SOURCE_DIR}/widgets/QMultiSelectWidget \
		$${SOURCE_DIR}/widgets/QSearchLineEdit \
		$${SOURCE_DIR}/widgets/QTextBrowserDialog

# Qxt/
# $$QMAKE_INCDIR_QT/QtSql/private

CONFIG	+= qt 
#warn_off

#for static building un-comment the following two lines
#CONFIG	+= static
#DEFINES += STATIC

QT += sql network

# Input
HEADERS += $${SOURCE_DIR}/mainwindow.h \
	$${SOURCE_DIR}/SearchItemDelegate.h \
	$${SOURCE_DIR}/SaagharWidget.h \
	$${SOURCE_DIR}/QGanjoorDbBrowser.h \
	$${SOURCE_DIR}/QGanjoorDbStuff.h \
	$${SOURCE_DIR}/settings.h \
	$${SOURCE_DIR}/SearchResultWidget.h \
	$${SOURCE_DIR}/SearchPatternManager.h \
	$${SOURCE_DIR}/version.h

FORMS += $${SOURCE_DIR}/mainwindow.ui \
	$${SOURCE_DIR}/settings.ui

SOURCES += $${SOURCE_DIR}/main.cpp \
	$${SOURCE_DIR}/mainwindow.cpp \
	$${SOURCE_DIR}/SearchItemDelegate.cpp \
	$${SOURCE_DIR}/SaagharWidget.cpp \
	$${SOURCE_DIR}/QGanjoorDbBrowser.cpp \
	$${SOURCE_DIR}/settings.cpp \
	$${SOURCE_DIR}/SearchResultWidget.cpp \
	$${SOURCE_DIR}/SearchPatternManager.cpp


include($${SOURCE_DIR}/widgets/QMultiSelectWidget/QMultiSelectWidget.pri)
include($${SOURCE_DIR}/widgets/QSearchLineEdit/QSearchLineEdit.pri)
include($${SOURCE_DIR}/widgets/QTextBrowserDialog/QTextBrowserDialog.pri)



#########################################
##for embeding SQlite and its Qt Driver
# DEFINES += EMBEDDED_SQLITE
# HEADERS		+= sqlite-driver/qsql_sqlite.h \
	# sqlite-driver/qsqlcachedresult_p.h
# SOURCES		+= sqlite-driver/qsql_sqlite.cpp \
	# sqlite-driver/qsqlcachedresult.cpp
 
# DEFINES += SQLITE_OMIT_LOAD_EXTENSION SQLITE_OMIT_COMPLETE
# DEFINES += SQLITE_ENABLE_FTS3 SQLITE_ENABLE_FTS3_PARENTHESIS

# HEADERS += sqlite-driver/sqlite/sqlite3.h \
	# sqlite-driver/sqlite/sqlite3ext.h
# SOURCES += sqlite-driver/sqlite/sqlite3.c
#########################################
# HEADERS		+= $$QMAKE_INCDIR_QT/../src/sql/drivers/sqlite/qsql_sqlite.h
# SOURCES		+= $$QMAKE_INCDIR_QT/../src/sql/drivers/sqlite/qsql_sqlite.cpp


RESOURCES += $${SOURCE_DIR}/saaghar.qrc

TRANSLATIONS += $${SOURCE_DIR}/saaghar_fa.ts

!build_pass:message("'make install' doesn't overwrite existing 'database file', do that manually!")

win32 {

win32-msvc*{
	DEFINES += D_MSVC_CC
	#QTPLUGIN += qsqlite
}

win32-g++{
	DEFINES += D_MINGW_CC
}

RC_FILE = $${SOURCE_DIR}/win.rc
	
target.path = Saaghar-Win
INSTALLS = target
utilities.path = Saaghar-Win
utilities.files = $${SOURCE_DIR}/utilities/AUTHORS \
	$${SOURCE_DIR}/utilities/CHANGELOG \
	$${SOURCE_DIR}/utilities/COPYING \
	$${SOURCE_DIR}/utilities/README \
	$${SOURCE_DIR}/utilities/TODO \
	$${SOURCE_DIR}/utilities/Saaghar-Manual.pdf \
	$${SOURCE_DIR}/utilities/saaghar_fa.qm \
	$${SOURCE_DIR}/utilities/qt_fa.qm \
	$${SOURCE_DIR}/saaghar.ico

poetsImage.path = Saaghar-Win/poets_images
poetsImage.files = $${SOURCE_DIR}/utilities/poets_images/10.png \
	$${SOURCE_DIR}/utilities/poets_images/11.png \
	$${SOURCE_DIR}/utilities/poets_images/12.png \
	$${SOURCE_DIR}/utilities/poets_images/13.png \
	$${SOURCE_DIR}/utilities/poets_images/14.png \
	$${SOURCE_DIR}/utilities/poets_images/15.png \
	$${SOURCE_DIR}/utilities/poets_images/16.png \
	$${SOURCE_DIR}/utilities/poets_images/17.png \
	$${SOURCE_DIR}/utilities/poets_images/18.png \
	$${SOURCE_DIR}/utilities/poets_images/19.png \
	$${SOURCE_DIR}/utilities/poets_images/2.png \
	$${SOURCE_DIR}/utilities/poets_images/20.png \
	$${SOURCE_DIR}/utilities/poets_images/21.png \
	$${SOURCE_DIR}/utilities/poets_images/22.png \
	$${SOURCE_DIR}/utilities/poets_images/23.png \
	$${SOURCE_DIR}/utilities/poets_images/24.png \
	$${SOURCE_DIR}/utilities/poets_images/25.png \
	$${SOURCE_DIR}/utilities/poets_images/26.png \
	$${SOURCE_DIR}/utilities/poets_images/27.png \
	$${SOURCE_DIR}/utilities/poets_images/28.png \
	$${SOURCE_DIR}/utilities/poets_images/29.png \
	$${SOURCE_DIR}/utilities/poets_images/3.png \
	$${SOURCE_DIR}/utilities/poets_images/30.png \
	$${SOURCE_DIR}/utilities/poets_images/31.png \
	$${SOURCE_DIR}/utilities/poets_images/32.png \
	$${SOURCE_DIR}/utilities/poets_images/33.png \
	$${SOURCE_DIR}/utilities/poets_images/34.png \
	$${SOURCE_DIR}/utilities/poets_images/35.png \
	$${SOURCE_DIR}/utilities/poets_images/37.png \
	$${SOURCE_DIR}/utilities/poets_images/38.png \
	$${SOURCE_DIR}/utilities/poets_images/39.png \
	$${SOURCE_DIR}/utilities/poets_images/4.png \
	$${SOURCE_DIR}/utilities/poets_images/40.png \
	$${SOURCE_DIR}/utilities/poets_images/41.png \
	$${SOURCE_DIR}/utilities/poets_images/42.png \
	$${SOURCE_DIR}/utilities/poets_images/43.png \
	$${SOURCE_DIR}/utilities/poets_images/44.png \
	$${SOURCE_DIR}/utilities/poets_images/45.png \
	$${SOURCE_DIR}/utilities/poets_images/46.png \
	$${SOURCE_DIR}/utilities/poets_images/47.png \
	$${SOURCE_DIR}/utilities/poets_images/5.png \
	$${SOURCE_DIR}/utilities/poets_images/501.png \
	$${SOURCE_DIR}/utilities/poets_images/502.png \
	$${SOURCE_DIR}/utilities/poets_images/503.png \
	$${SOURCE_DIR}/utilities/poets_images/504.png \
	$${SOURCE_DIR}/utilities/poets_images/505.png \
	$${SOURCE_DIR}/utilities/poets_images/6.png \
	$${SOURCE_DIR}/utilities/poets_images/600.png \
	$${SOURCE_DIR}/utilities/poets_images/602.png \
	$${SOURCE_DIR}/utilities/poets_images/7.png \
	$${SOURCE_DIR}/utilities/poets_images/8.png \
	$${SOURCE_DIR}/utilities/poets_images/9.png \
	$${SOURCE_DIR}/utilities/poets_images/48.png \
	$${SOURCE_DIR}/utilities/poets_images/49.png \
	$${SOURCE_DIR}/utilities/poets_images/50.png \
	$${SOURCE_DIR}/utilities/poets_images/506.png \
	$${SOURCE_DIR}/utilities/poets_images/51.png \
	$${SOURCE_DIR}/utilities/poets_images/610.png

INSTALLS += utilities \
	poetsImage
}

mac {
LOG = $$system(mkdir ~/Library/)
LOG += $$system(mkdir ~/Library/Saaghar)
LOG += $$system(cp -n $${SOURCE_DIR}/utilities/* ~/Library/Saaghar/)

CONFIG += link_prl x86
QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.5.sdk
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5

target.path = /Applications
INSTALLS = target
utilities.path = Contents/Resources
utilities.files = $${SOURCE_DIR}/utilities/AUTHORS \
	$${SOURCE_DIR}/utilities/CHANGELOG \
	$${SOURCE_DIR}/utilities/COPYING \
	$${SOURCE_DIR}/utilities/README \
	$${SOURCE_DIR}/utilities/Saaghar-Manual.pdf \
	$${SOURCE_DIR}/utilities/saaghar_fa.qm \
	$${SOURCE_DIR}/utilities/qt_fa.qm \
	$${SOURCE_DIR}/utilities/TODO

poetsImage.path = Contents/Resources/poets_images
poetsImage.files = $${SOURCE_DIR}/utilities/poets_images/10.png \
	$${SOURCE_DIR}/utilities/poets_images/11.png \
	$${SOURCE_DIR}/utilities/poets_images/12.png \
	$${SOURCE_DIR}/utilities/poets_images/13.png \
	$${SOURCE_DIR}/utilities/poets_images/14.png \
	$${SOURCE_DIR}/utilities/poets_images/15.png \
	$${SOURCE_DIR}/utilities/poets_images/16.png \
	$${SOURCE_DIR}/utilities/poets_images/17.png \
	$${SOURCE_DIR}/utilities/poets_images/18.png \
	$${SOURCE_DIR}/utilities/poets_images/19.png \
	$${SOURCE_DIR}/utilities/poets_images/2.png \
	$${SOURCE_DIR}/utilities/poets_images/20.png \
	$${SOURCE_DIR}/utilities/poets_images/21.png \
	$${SOURCE_DIR}/utilities/poets_images/22.png \
	$${SOURCE_DIR}/utilities/poets_images/23.png \
	$${SOURCE_DIR}/utilities/poets_images/24.png \
	$${SOURCE_DIR}/utilities/poets_images/25.png \
	$${SOURCE_DIR}/utilities/poets_images/26.png \
	$${SOURCE_DIR}/utilities/poets_images/27.png \
	$${SOURCE_DIR}/utilities/poets_images/28.png \
	$${SOURCE_DIR}/utilities/poets_images/29.png \
	$${SOURCE_DIR}/utilities/poets_images/3.png \
	$${SOURCE_DIR}/utilities/poets_images/30.png \
	$${SOURCE_DIR}/utilities/poets_images/31.png \
	$${SOURCE_DIR}/utilities/poets_images/32.png \
	$${SOURCE_DIR}/utilities/poets_images/33.png \
	$${SOURCE_DIR}/utilities/poets_images/34.png \
	$${SOURCE_DIR}/utilities/poets_images/35.png \
	$${SOURCE_DIR}/utilities/poets_images/37.png \
	$${SOURCE_DIR}/utilities/poets_images/38.png \
	$${SOURCE_DIR}/utilities/poets_images/39.png \
	$${SOURCE_DIR}/utilities/poets_images/4.png \
	$${SOURCE_DIR}/utilities/poets_images/40.png \
	$${SOURCE_DIR}/utilities/poets_images/41.png \
	$${SOURCE_DIR}/utilities/poets_images/42.png \
	$${SOURCE_DIR}/utilities/poets_images/43.png \
	$${SOURCE_DIR}/utilities/poets_images/44.png \
	$${SOURCE_DIR}/utilities/poets_images/45.png \
	$${SOURCE_DIR}/utilities/poets_images/46.png \
	$${SOURCE_DIR}/utilities/poets_images/47.png \
	$${SOURCE_DIR}/utilities/poets_images/5.png \
	$${SOURCE_DIR}/utilities/poets_images/501.png \
	$${SOURCE_DIR}/utilities/poets_images/502.png \
	$${SOURCE_DIR}/utilities/poets_images/503.png \
	$${SOURCE_DIR}/utilities/poets_images/504.png \
	$${SOURCE_DIR}/utilities/poets_images/505.png \
	$${SOURCE_DIR}/utilities/poets_images/6.png \
	$${SOURCE_DIR}/utilities/poets_images/600.png \
	$${SOURCE_DIR}/utilities/poets_images/602.png \
	$${SOURCE_DIR}/utilities/poets_images/7.png \
	$${SOURCE_DIR}/utilities/poets_images/8.png \
	$${SOURCE_DIR}/utilities/poets_images/9.png \
	$${SOURCE_DIR}/utilities/poets_images/48.png \
	$${SOURCE_DIR}/utilities/poets_images/49.png \
	$${SOURCE_DIR}/utilities/poets_images/50.png \
	$${SOURCE_DIR}/utilities/poets_images/506.png \
	$${SOURCE_DIR}/utilities/poets_images/51.png \
	$${SOURCE_DIR}/utilities/poets_images/610.png
 
QMAKE_BUNDLE_DATA += utilities\
                     poetsImage
ICON = $${SOURCE_DIR}/saaghar.icns
QMAKE_INFO_PLIST = $${SOURCE_DIR}/Info.plist

!build_pass:message("'make uninstall' doesn't remove "$$(HOME)/Library/Saaghar" directory and its contents especially 'database file', do that manually!")
}

unix:!macx {
TARGET = saaghar
LOG = $$system(mkdir ~/.Pojh)
LOG += $$system(mkdir ~/.Pojh/Saaghar)
LOG  += $$system(cp -n $${SOURCE_DIR}/utilities/* ~/.Pojh/Saaghar/)

PREFIX = /usr
DESKTOPDIR = $${PREFIX}/share/applications
ICONDIR = $${PREFIX}/share/pixmaps

target.path = $${PREFIX}/bin
INSTALLS = target

utilities.path = $${PREFIX}/share/saaghar
utilities.files = $${SOURCE_DIR}/utilities/AUTHORS \
	$${SOURCE_DIR}/utilities/CHANGELOG \
	$${SOURCE_DIR}/utilities/COPYING \
	$${SOURCE_DIR}/utilities/README \
	$${SOURCE_DIR}/utilities/Saaghar-Manual.pdf \
	$${SOURCE_DIR}/utilities/saaghar_fa.qm \
	$${SOURCE_DIR}/utilities/qt_fa.qm \
	$${SOURCE_DIR}/utilities/TODO

poetsImage.path = $${PREFIX}/share/saaghar/poets_images
poetsImage.files = $${SOURCE_DIR}/utilities/poets_images/10.png \
	$${SOURCE_DIR}/utilities/poets_images/11.png \
	$${SOURCE_DIR}/utilities/poets_images/12.png \
	$${SOURCE_DIR}/utilities/poets_images/13.png \
	$${SOURCE_DIR}/utilities/poets_images/14.png \
	$${SOURCE_DIR}/utilities/poets_images/15.png \
	$${SOURCE_DIR}/utilities/poets_images/16.png \
	$${SOURCE_DIR}/utilities/poets_images/17.png \
	$${SOURCE_DIR}/utilities/poets_images/18.png \
	$${SOURCE_DIR}/utilities/poets_images/19.png \
	$${SOURCE_DIR}/utilities/poets_images/2.png \
	$${SOURCE_DIR}/utilities/poets_images/20.png \
	$${SOURCE_DIR}/utilities/poets_images/21.png \
	$${SOURCE_DIR}/utilities/poets_images/22.png \
	$${SOURCE_DIR}/utilities/poets_images/23.png \
	$${SOURCE_DIR}/utilities/poets_images/24.png \
	$${SOURCE_DIR}/utilities/poets_images/25.png \
	$${SOURCE_DIR}/utilities/poets_images/26.png \
	$${SOURCE_DIR}/utilities/poets_images/27.png \
	$${SOURCE_DIR}/utilities/poets_images/28.png \
	$${SOURCE_DIR}/utilities/poets_images/29.png \
	$${SOURCE_DIR}/utilities/poets_images/3.png \
	$${SOURCE_DIR}/utilities/poets_images/30.png \
	$${SOURCE_DIR}/utilities/poets_images/31.png \
	$${SOURCE_DIR}/utilities/poets_images/32.png \
	$${SOURCE_DIR}/utilities/poets_images/33.png \
	$${SOURCE_DIR}/utilities/poets_images/34.png \
	$${SOURCE_DIR}/utilities/poets_images/35.png \
	$${SOURCE_DIR}/utilities/poets_images/37.png \
	$${SOURCE_DIR}/utilities/poets_images/38.png \
	$${SOURCE_DIR}/utilities/poets_images/39.png \
	$${SOURCE_DIR}/utilities/poets_images/4.png \
	$${SOURCE_DIR}/utilities/poets_images/40.png \
	$${SOURCE_DIR}/utilities/poets_images/41.png \
	$${SOURCE_DIR}/utilities/poets_images/42.png \
	$${SOURCE_DIR}/utilities/poets_images/43.png \
	$${SOURCE_DIR}/utilities/poets_images/44.png \
	$${SOURCE_DIR}/utilities/poets_images/45.png \
	$${SOURCE_DIR}/utilities/poets_images/46.png \
	$${SOURCE_DIR}/utilities/poets_images/47.png \
	$${SOURCE_DIR}/utilities/poets_images/5.png \
	$${SOURCE_DIR}/utilities/poets_images/501.png \
	$${SOURCE_DIR}/utilities/poets_images/502.png \
	$${SOURCE_DIR}/utilities/poets_images/503.png \
	$${SOURCE_DIR}/utilities/poets_images/504.png \
	$${SOURCE_DIR}/utilities/poets_images/505.png \
	$${SOURCE_DIR}/utilities/poets_images/6.png \
	$${SOURCE_DIR}/utilities/poets_images/600.png \
	$${SOURCE_DIR}/utilities/poets_images/602.png \
	$${SOURCE_DIR}/utilities/poets_images/7.png \
	$${SOURCE_DIR}/utilities/poets_images/8.png \
	$${SOURCE_DIR}/utilities/poets_images/9.png \
	$${SOURCE_DIR}/utilities/poets_images/48.png \
	$${SOURCE_DIR}/utilities/poets_images/49.png \
	$${SOURCE_DIR}/utilities/poets_images/50.png \
	$${SOURCE_DIR}/utilities/poets_images/506.png \
	$${SOURCE_DIR}/utilities/poets_images/51.png \
	$${SOURCE_DIR}/utilities/poets_images/610.png

desktop.path = $${DESKTOPDIR}
desktop.files = $${SOURCE_DIR}/utilities/Saaghar.desktop
INSTALLS += desktop \
			poetsImage \
			utilities

icon.path = $${ICONDIR}
icon.files = $${SOURCE_DIR}/images/saaghar.png
INSTALLS += icon

!build_pass:message("'make uninstall' doesn't remove "$$(HOME)/.Pojh/Saaghar" directory and its contents especially 'database file', do that manually!")
}
