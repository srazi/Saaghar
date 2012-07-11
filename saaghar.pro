TEMPLATE = app
TARGET = Saaghar

isEmpty( SOURCE_DIR ) {
SOURCE_DIR = .
}


DEPENDPATH += $${SOURCE_DIR}/. \
			$${SOURCE_DIR}/build

INCLUDEPATH += $${SOURCE_DIR}/.

# $${SOURCE_DIR}/sqlite $${SOURCE_DIR}/sqlite-driver/sqlite


# $$QMAKE_INCDIR_QT/QtSql/private

CONFIG	+= qt 
#warn_off

##un-comment the following two lines for skipping all warning and debug messages.
CONFIG(debug, debug|release) {
	!build_pass:message("DEBUG BUILD")
} else {
	!build_pass:message("RELEASE BUILD")

	DEFINES += QT_NO_WARNING_OUTPUT
	DEFINES += QT_NO_DEBUG_OUTPUT
}

##for static building un-comment the following two lines
#CONFIG	+= static
#DEFINES += STATIC

QT += sql network xml

# Input
HEADERS += $${SOURCE_DIR}/mainwindow.h \
	$${SOURCE_DIR}/SearchItemDelegate.h \
	$${SOURCE_DIR}/SaagharWidget.h \
	$${SOURCE_DIR}/QGanjoorDbBrowser.h \
	$${SOURCE_DIR}/QGanjoorDbStuff.h \
	$${SOURCE_DIR}/settings.h \
	$${SOURCE_DIR}/SearchResultWidget.h \
	$${SOURCE_DIR}/SearchPatternManager.h \
	$${SOURCE_DIR}/version.h \
	$${SOURCE_DIR}/bookmarks.h \
	$${SOURCE_DIR}/commands.h \
	$${SOURCE_DIR}/outline.h \
	$${SOURCE_DIR}/DataBaseUpdater.h \
	$${SOURCE_DIR}/NoDataBaseDialog.h

FORMS += $${SOURCE_DIR}/mainwindow.ui \
	$${SOURCE_DIR}/settings.ui \
	$${SOURCE_DIR}/databaseupdater.ui \
	$${SOURCE_DIR}/nodatabasedialog.ui

SOURCES += $${SOURCE_DIR}/main.cpp \
	$${SOURCE_DIR}/mainwindow.cpp \
	$${SOURCE_DIR}/SearchItemDelegate.cpp \
	$${SOURCE_DIR}/SaagharWidget.cpp \
	$${SOURCE_DIR}/QGanjoorDbBrowser.cpp \
	$${SOURCE_DIR}/settings.cpp \
	$${SOURCE_DIR}/SearchResultWidget.cpp \
	$${SOURCE_DIR}/SearchPatternManager.cpp \
	$${SOURCE_DIR}/bookmarks.cpp \
	$${SOURCE_DIR}/commands.cpp \
	$${SOURCE_DIR}/outline.cpp \
	$${SOURCE_DIR}/DataBaseUpdater.cpp \
	$${SOURCE_DIR}/NoDataBaseDialog.cpp

include($${SOURCE_DIR}/pQjWidgets/pQjWidgets.pri)
include($${SOURCE_DIR}/qmusicplayer/qmusicplayer.pri)
include($${SOURCE_DIR}/downloader/downloader.pri)
include($${SOURCE_DIR}/OSDaB-Zip/osdabzip.pri)


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

RESOURCES_PATH = Saaghar-Win

utilities.files = $${SOURCE_DIR}/saaghar.ico

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
INSTALLS = target

RESOURCES_PATH = Contents/Resources

ICON = $${SOURCE_DIR}/saaghar.icns
QMAKE_INFO_PLIST = $${SOURCE_DIR}/Info.plist

#!build_pass:message("'make uninstall' doesn't remove "$$(HOME)/Library/Saaghar" directory and its contents especially 'database file', do that manually!")
}

unix:!macx {
TARGET = saaghar

isEmpty( PREFIX ) {
PREFIX = /usr
}

DESKTOPDIR = $${PREFIX}/share/applications
ICONDIR = $${PREFIX}/share/pixmaps

target.path = $${PREFIX}/bin
INSTALLS = target

RESOURCES_PATH = $${PREFIX}/share/saaghar

desktop.path = $${DESKTOPDIR}
desktop.files = $${SOURCE_DIR}/utilities/Saaghar.desktop

icon.path = $${ICONDIR}
icon.files = $${SOURCE_DIR}/images/saaghar.png

INSTALLS += desktop \
			icon

#!build_pass:message("'make uninstall' doesn't remove "$$(HOME)/.Pojh/Saaghar" directory and its contents especially 'database file', do that manually!")
}

##installation files
utilities.path = $${RESOURCES_PATH}
utilities.files += $${SOURCE_DIR}/utilities/AUTHORS \
	$${SOURCE_DIR}/utilities/CHANGELOG \
	$${SOURCE_DIR}/utilities/COPYING \
	$${SOURCE_DIR}/utilities/README \
	$${SOURCE_DIR}/utilities/Saaghar-Manual.pdf \
	$${SOURCE_DIR}/utilities/saaghar_fa.qm \
	$${SOURCE_DIR}/utilities/qt_fa.qm \
	$${SOURCE_DIR}/utilities/TODO \
	$${SOURCE_DIR}/utilities/LICENSE

poetsImage.path = $${RESOURCES_PATH}/poets_images
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

backgrounds.path = $${RESOURCES_PATH}/themes/backgrounds
backgrounds.files = $${SOURCE_DIR}/utilities/themes/backgrounds/bgpatterns_1.png \
	$${SOURCE_DIR}/utilities/themes/backgrounds/bgpatterns_2.png \
	$${SOURCE_DIR}/utilities/themes/backgrounds/bgpatterns_3.png \
	$${SOURCE_DIR}/utilities/themes/backgrounds/bgpatterns_4.png \
	$${SOURCE_DIR}/utilities/themes/backgrounds/grav-rand_1.png \
	$${SOURCE_DIR}/utilities/themes/backgrounds/grav-rand_2.png \
	$${SOURCE_DIR}/utilities/themes/backgrounds/grav-rand_3.png \
	$${SOURCE_DIR}/utilities/themes/backgrounds/grav-rand_4.png \
	$${SOURCE_DIR}/utilities/themes/backgrounds/saaghar-pattern_1.png \
	$${SOURCE_DIR}/utilities/themes/backgrounds/woodw-blue_1.png \
	$${SOURCE_DIR}/utilities/themes/backgrounds/woodw-golden_1.png \
	$${SOURCE_DIR}/utilities/themes/backgrounds/woodw-golden_2.png \
	$${SOURCE_DIR}/utilities/themes/backgrounds/woodw-green_1.png \
	$${SOURCE_DIR}/utilities/themes/backgrounds/woodw_3d_1.png

lightGrayIcons.path = $${RESOURCES_PATH}/themes/iconsets/light-gray
lightGrayIcons.files = $${SOURCE_DIR}/utilities/themes/iconsets/light-gray/browse_net.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/cancel.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/check-updates.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/clear-left.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/clear-right.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/close-tab.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/copy.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/down.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/exit.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/export.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/export-pdf.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/faal.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/filter.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/fullscreen.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/home.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/import-to-database.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/left.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/new_tab.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/new_window.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/next.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/no-fullscreen.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/ocr-verification.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/previous.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/print.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/print-preview.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/qt-logo.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/random.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/README \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/remove-poet.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/right.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/search.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/search-options.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/select-mask.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/settings.png \
	$${SOURCE_DIR}/utilities/themes/iconsets/light-gray/up.png


exists( $${SOURCE_DIR}/OTHERS/fonts/*.ttf ) {
	fonts.path = $${RESOURCES_PATH}/fonts
	fonts.files = $$quote($${SOURCE_DIR}/OTHERS/fonts/XB Sols.ttf) \
				$${SOURCE_DIR}/OTHERS/fonts/license.txt
	mac {
		QMAKE_BUNDLE_DATA += fonts
	}
	else {
		INSTALLS += fonts
	}
}

mac {
	QMAKE_BUNDLE_DATA += utilities\
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
