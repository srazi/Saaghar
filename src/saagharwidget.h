/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2015 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pozh.org>             *
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 3 of the License,         *
 *  (at your option) any later version                                     *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details                            *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program; if not, see http://www.gnu.org/licenses/      *
 *                                                                         *
 ***************************************************************************/

#ifndef SAAGHARWIDGET_H
#define SAAGHARWIDGET_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPushButton>
#include <QToolBar>
#include <QUndoStack>

#include "databaseelements.h"
#include "bookmarks.h"

class DatabaseBrowser;
class QSearchLineEdit;
class QTextEdit;
class QSplitter;

#ifdef MEDIA_PLAYER
class QMusicPlayer;
#endif

class SaagharWidget : public QWidget
{
    Q_OBJECT

public:
    SaagharWidget(QWidget* parent, QToolBar* catsToolBar, QTableWidget* tableWidget);
    ~SaagharWidget();

#ifdef MEDIA_PLAYER
    static QMusicPlayer* musicPlayer;
#endif

    enum PoemViewStyle {
        TwoHemistichLine,
        OneHemistichLine,
        SteppedHemistichLine
    };

    enum PageType {
        CategoryViewerPage,
        PoemViewerPage
    };

    struct MetaInfo {
        PageType type;
        int id;
        //QString mediaFile;
    } pageMetaInfo;

    QString currentCaption;
    QPushButton* parentCatButton;
    QTableWidget* tableViewWidget;
    void applyDefaultSectionsHeight();
    void resizeTable(QTableWidget* table);
    bool nextPoem();
    bool previousPoem();
    void clearSaagharWidget();
    QToolBar* parentCatsToolBar;
    void showHome();
    QString currentPageGanjoorUrl();
    void loadSettings();
    void showParentCategory(GanjoorCat category);
    void processClickedItem(QString type, int id, bool error, bool pushToStack = true);
    void navigateToPage(QString type, int id, bool error);

    int currentVerticalPosition();
    void setVerticalPosition(int vPosition);

    QString highlightCell(int vorder);

    int minMesraWidth;

    //STATIC Variables
    static QString poetsImagesDir;
    static QLocale  persianIranLocal;
    //static QFont tableFont;
    static bool showBeytNumbers;
    static bool backgroundImageState;
    static PoemViewStyle CurrentViewStyle;
    //static bool newSearchFlag;
    //static bool newSearchSkipNonAlphabet;
    static QString backgroundImagePath;
    //static QColor textColor;
    static QColor matchedTextColor;
    static QColor backgroundColor;
    static QTableWidgetItem* lastOveredItem;
    static int maxPoetsPerGroup;

    //bookmark widget
    static Bookmarks* bookmarks;

    //search field object
    static QSearchLineEdit* lineEditSearchText;

    //DataBase
    static int computeRowHeight(const QFontMetrics &fontMetric, int textWidth, int width, int height = 0);

    static QHash<int, QPair<QString, qint64> > mediaInfoCash;
    static QHash<int, QString> longestHemistiches;

    int currentPoem;
    int currentCat;
    int currentParentID;
    QStringList currentLocationList;
    QString currentPoemTitle;

    void homeResizeColsRows();
    inline bool isDirty()
    {return dirty;}
    inline void setDirty()
    {dirty = true;}
    QStringList identifier();
    void refresh();

    inline void setMVPosition(int value) { m_vPosition = value; }

    //Undo FrameWork
    QUndoStack* undoStack;

private:
    void doPoemLayout(int* prow, QTableWidgetItem* mesraItem, const QString &currentVerseText, const QFontMetrics &fontMetric, VersePosition versePosition/*, Qt::Alignment beytAlignment*/);
    QTextEdit* createItemForLongText(int row, int column, const QString &text = "", const QString &highlightText = "");
    bool initializeCustomizedHome();
    QMap<int, QPair<int, int> > rowParagraphHeightMap;
    QMap<int, int> rowSingleHeightMap;
    void showCategory(GanjoorCat category);
    void showPoem(GanjoorPoem poem);
    QPoint pressedPosition;
    bool dirty;
    int m_vPosition;

private slots:
    void createCustomContextMenu(const QPoint &pos);
    void parentCatClicked();
    void clickedOnItem(int row, int col);
    void pressedOnItem(int row, int col);

    void setFromMVPosition();

#ifdef Q_OS_MAC
    void forceReLayoutTable();
#endif

public slots:
    QTableWidgetItem* scrollToFirstItemContains(const QString &phrase, bool pharseIsList = true, bool scroll = true);

signals:
    void captionChanged();
    void navNextActionState(bool);
    void navPreviousActionState(bool);
    void loadingStatusText(const QString &, int num = 0);
    void createContextMenuRequested(const QPoint &);
};
#endif // SAAGHARWIDGET_H
