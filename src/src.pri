DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

QT += sql network xml

# Input
HEADERS += \
    $$PWD/saagharwindow.h \
    $$PWD/searchitemdelegate.h \
    $$PWD/saagharwidget.h \
    $$PWD/databasebrowser.h \
    $$PWD/databaseelements.h \
    $$PWD/settings.h \
    $$PWD/searchresultwidget.h \
    $$PWD/searchpatternmanager.h \
    $$PWD/version.h \
    $$PWD/bookmarks.h \
    $$PWD/commands.h \
    $$PWD/outline.h \
    $$PWD/databaseupdater.h \
    $$PWD/nodatabasedialog.h \
    $$PWD/qtwin.h \
    $$PWD/searchoptionsdialog.h \
    $$PWD/tabwidget.h \
    $$PWD/mactoolbutton.h \
    $$PWD/tools.h \
    $$PWD/concurrenttasks.h \
    $$PWD/saagharapplication.h \
    $$PWD/settingsmanager.h \
    $$PWD/outlinemodel.h \
    $$PWD/selectionmanager.h \
    $$PWD/audiorepodownloader.h \
    $$PWD/importer/importermanager.h \
    $$PWD/importer/importer_interface.h \
    $$PWD/importer/txtimporter.h

FORMS += \
    $$PWD/saagharwindow.ui \
    $$PWD/settings.ui \
    $$PWD/databaseupdater.ui \
    $$PWD/nodatabasedialog.ui \
    $$PWD/searchoptionsdialog.ui \
    $$PWD/selectionmanager.ui \
    $$PWD/audiorepodownloader.ui

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/saagharwindow.cpp \
    $$PWD/searchitemdelegate.cpp \
    $$PWD/saagharwidget.cpp \
    $$PWD/databasebrowser.cpp \
    $$PWD/settings.cpp \
    $$PWD/searchresultwidget.cpp \
    $$PWD/searchpatternmanager.cpp \
    $$PWD/bookmarks.cpp \
    $$PWD/commands.cpp \
    $$PWD/outline.cpp \
    $$PWD/databaseupdater.cpp \
    $$PWD/nodatabasedialog.cpp \
    $$PWD/qtwin.cpp \
    $$PWD/searchoptionsdialog.cpp \
    $$PWD/tabwidget.cpp \
    $$PWD/mactoolbutton.cpp \
    $$PWD/tools.cpp \
    $$PWD/concurrenttasks.cpp \
    $$PWD/saagharapplication.cpp \
    $$PWD/settingsmanager.cpp \
    $$PWD/outlinemodel.cpp \
    $$PWD/selectionmanager.cpp \
    $$PWD/audiorepodownloader.cpp \
    $$PWD/importer/importermanager.cpp \
    $$PWD/importer/txtimporter.cpp

include(pQjWidgets/pqjwidgets.pri)
include(downloader/downloader.pri)
include(OSDaB-Zip/osdabzip.pri)
include(progressmanager/progressmanager.pri)
include(QIrBreadCrumbBar/QIrBreadCrumbBar.pri)

contains( DEFINES, MEDIA_PLAYER ) {
    include(qmusicplayer/qmusicplayer.pri)
}

#########################################
##for embeding SQlite and its Qt Driver
# DEFINES += EMBEDDED_SQLITE
# HEADERS += sqlite-driver/qsql_sqlite.h \
    # sqlite-driver/qsqlcachedresult_p.h
# SOURCES += sqlite-driver/qsql_sqlite.cpp \
    # sqlite-driver/qsqlcachedresult.cpp

# DEFINES += SQLITE_OMIT_LOAD_EXTENSION SQLITE_OMIT_COMPLETE
# DEFINES += SQLITE_ENABLE_FTS3 SQLITE_ENABLE_FTS3_PARENTHESIS

# HEADERS += sqlite-driver/sqlite/sqlite3.h \
    # sqlite-driver/sqlite/sqlite3ext.h
# SOURCES += sqlite-driver/sqlite/sqlite3.c
#########################################
# HEADERS += $$QMAKE_INCDIR_QT/../src/sql/drivers/sqlite/qsql_sqlite.h
# SOURCES += $$QMAKE_INCDIR_QT/../src/sql/drivers/sqlite/qsql_sqlite.cpp
