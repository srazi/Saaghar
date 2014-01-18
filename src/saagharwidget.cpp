/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2014 by S. Razi Alavizadeh                          *
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

#include "searchpatternmanager.h"
#include "searchitemdelegate.h"
#include "saagharwidget.h"
#include "commands.h"

#include <QSearchLineEdit>
#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QScrollBar>
#include <QMessageBox>
#include <QAction>
#include <QFile>
#include <QTextEdit>
#include <QSplitter>

//STATIC Variables
QString SaagharWidget::poetsImagesDir = QString();
QLocale  SaagharWidget::persianIranLocal = QLocale();
//QFont SaagharWidget::tableFont = qApp->font();
bool SaagharWidget::showBeytNumbers = true;
bool SaagharWidget::backgroundImageState = false;
SaagharWidget::PoemViewStyle SaagharWidget::CurrentViewStyle = SaagharWidget::SteppedHemistichLine;
//bool SaagharWidget::newSearchFlag = false;
//bool SaagharWidget::newSearchSkipNonAlphabet = false;
QString SaagharWidget::backgroundImagePath = QString();
//QColor SaagharWidget::textColor = QColor();
QColor SaagharWidget::matchedTextColor = QColor();
QColor SaagharWidget::backgroundColor = QColor();
QTableWidgetItem* SaagharWidget::lastOveredItem = 0;
int SaagharWidget::maxPoetsPerGroup = 12;
QHash<int, QPair<QString, qint64> > SaagharWidget::mediaInfoCash = QHash<int, QPair<QString, qint64> >();
QHash<int, QString> SaagharWidget::longestHemistiches = QHash<int, QString>();

//bookmark widget
Bookmarks* SaagharWidget::bookmarks = 0;
//search field object
QSearchLineEdit* SaagharWidget::lineEditSearchText = 0;
//ganjoor data base browser
QGanjoorDbBrowser* SaagharWidget::ganjoorDataBase = NULL;

//Constants
const int ITEM_BOOKMARKED_STATE = Qt::UserRole + 20;
const Qt::ItemFlags numItemFlags = Qt::ItemIsEnabled;//Qt::NoItemFlags;
const Qt::ItemFlags catsItemFlag = Qt::ItemIsEnabled;
const Qt::ItemFlags poemsItemFlag = Qt::ItemIsEnabled;
const Qt::ItemFlags versesItemFlag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
const Qt::ItemFlags poemsTitleItemFlag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

#ifndef NO_PHONON_LIB
#include "qmusicplayer.h"
QMusicPlayer* SaagharWidget::musicPlayer = NULL;
#endif

SaagharWidget::SaagharWidget(QWidget* parent, QToolBar* catsToolBar, QTableWidget* tableWidget)
    : QWidget(parent)
    , tableViewWidget(tableWidget)
    , parentCatsToolBar(catsToolBar)
    , m_vPosition(-1)
    , currentPoem(0)
    , currentCat(0)
{
    pageMetaInfo.id = 0;
    pageMetaInfo.type = SaagharWidget::CategoryViewerPage;

    tableViewWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    dirty = true;
    minMesraWidth = 0;
    rowParagraphHeightMap.clear();
    rowSingleHeightMap.clear();

    //Undo FrameWork
    undoStack = new QUndoStack(this);

    loadSettings();

    currentCat = currentPoem = 0;
    currentParentID = 0;
    currentCaption = tr("Home");
    pressedPosition = QPoint(-1, -1);

    connect(this->tableViewWidget, SIGNAL(cellClicked(int,int)), this, SLOT(clickedOnItem(int,int)));
    connect(this->tableViewWidget, SIGNAL(cellPressed(int,int)), this, SLOT(pressedOnItem(int,int)));
    showHome();
}

SaagharWidget::~SaagharWidget()
{
}

void SaagharWidget::applyDefaultSectionsHeight()
{
    QHeaderView* header = tableViewWidget->verticalHeader();

    int bookmarkIconHeight = 0;
    if (SaagharWidget::bookmarks) {
#ifdef Q_OS_MAC
        bookmarkIconHeight = 25;//35
#else
        bookmarkIconHeight = 22;
#endif
    }

    int height;
    if (currentPoem == 0) {
        height = SaagharWidget::computeRowHeight(QFontMetrics(Settings::getFromFonts(Settings::SectionNameFontColor)), -1, -1);
    }
    else {
        height = qMax(SaagharWidget::computeRowHeight(QFontMetrics(Settings::getFromFonts(Settings::PoemTextFontColor)), -1, -1), bookmarkIconHeight * 5 / 4);
        if (SaagharWidget::showBeytNumbers) {
            height = qMax(height, SaagharWidget::computeRowHeight(QFontMetrics(Settings::getFromFonts(Settings::NumbersFontColor)), -1, -1));
        }
    }

    header->setDefaultSectionSize(height);
    tableViewWidget->setVerticalHeader(header);
}

void SaagharWidget::loadSettings()
{
    tableViewWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    //tableViewWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);
    QPalette p(tableViewWidget->palette());

#if defined(Q_OS_MAC) || defined(Q_OS_X11)
    if (SaagharWidget::backgroundImageState && !SaagharWidget::backgroundImagePath.isEmpty()) {
        p.setBrush(QPalette::Base, QBrush(QPixmap(SaagharWidget::backgroundImagePath)));
    }
    else {
        p.setColor(QPalette::Base, SaagharWidget::backgroundColor);
    }
    p.setColor(QPalette::Text, Settings::getFromColors(Settings::PoemTextFontColor) /*SaagharWidget::textColor*/);
#endif

    if (SaagharWidget::backgroundImageState && !SaagharWidget::backgroundImagePath.isEmpty()) {
        connect(tableViewWidget->verticalScrollBar()        , SIGNAL(valueChanged(int)), tableViewWidget->viewport(), SLOT(update()));
        connect(tableViewWidget->horizontalScrollBar()      , SIGNAL(valueChanged(int)), tableViewWidget->viewport(), SLOT(update()));
    }
    else {
        disconnect(tableViewWidget->verticalScrollBar()     , SIGNAL(valueChanged(int)), tableViewWidget->viewport(), SLOT(update()));
        disconnect(tableViewWidget->horizontalScrollBar()   , SIGNAL(valueChanged(int)), tableViewWidget->viewport(), SLOT(update()));
    }
    tableViewWidget->setPalette(p);
    //tableViewWidget->setFont(SaagharWidget::tableFont);

    QHeaderView* header = tableViewWidget->verticalHeader();
    header->hide();

    applyDefaultSectionsHeight();
}

void SaagharWidget::processClickedItem(QString type, int id, bool noError, bool pushToStack)
{
    if (!noError) {
        type = "CatID";
        id = 0;
    }
    if ((type == identifier().at(0)) && (id == identifier().at(1).toInt())) {
        navigateToPage(type, id, true);    //refresh and don't push to stack
    }
    else {
        if (pushToStack) {
            NavigateToPage* navigateCommand = new NavigateToPage(this, type, id);
            undoStack->push(navigateCommand);
        }
        else {
            navigateToPage(type, id, true);//don't push to stack
        }
    }
}

void SaagharWidget::navigateToPage(QString type, int id, bool noError)
{
    if (type == "PoemID" || type == "CatID") {
        clearSaagharWidget();
    }

    if (!noError) {
        type = "CatID";
    }
    if (type == "PoemID") {
        GanjoorPoem poem = ganjoorDataBase->getPoem(id);
        showPoem(poem);
    }
    else if (type == "CatID") {
        GanjoorCat category;
        category.init(0, 0, "", 0, "");
        if (noError) {
            category = ganjoorDataBase->getCategory(id);
        }
        showCategory(category);
    }

#ifndef NO_PHONON_LIB
    if (SaagharWidget::musicPlayer) {
        bool isEnabled = (pageMetaInfo.type == SaagharWidget::PoemViewerPage);
        SaagharWidget::musicPlayer->setEnabled(isEnabled);
        if (!isEnabled) {
            SaagharWidget::musicPlayer->stop();
        }
    }
#endif
}

int SaagharWidget::currentVerticalPosition()
{
    return tableViewWidget->verticalScrollBar()->value();
//    int centerY = tableViewWidget->viewport()->rect().center().y();
//    while (centerY >= 0 && tableViewWidget->rowAt(centerY) < 0) {
//        --centerY;
//    }

//    if (centerY >= 0 && tableViewWidget->rowAt(centerY) >= 0) {
//        return tableViewWidget->rowAt(centerY);
//    }
//    else {
//        return -1;
//    }
}

void SaagharWidget::setVerticalPosition(int vPosition)
{
    tableViewWidget->verticalScrollBar()->setValue(vPosition);
//    if (row <= 0) {
//        tableViewWidget->verticalScrollBar()->setValue(0);
//    }
//    else if (row >= tableViewWidget->rowCount() - 1) {
//        tableViewWidget->verticalScrollBar()->setValue(tableViewWidget->verticalScrollBar()->maximum());
//    }

//    int columnCount = tableViewWidget->columnCount();
//    for (int i = row; i > 0; --i) {
//        for (int col = 0; col < columnCount; ++col) {
//            QTableWidgetItem* item = tableViewWidget->item(i, col);
//            if (item) {
//                tableViewWidget->scrollToItem(item, QAbstractItemView::PositionAtCenter);
//                return;
//            }
//        }
    //    }
}

QString SaagharWidget::highlightCell(int vorder)
{
    QString text;
    if (currentPoem <= 0) {
        return text;
    }

    int numOfCols = tableViewWidget->columnCount();
    int numOfRows = tableViewWidget->rowCount();
    const QColor poemColor(Settings::getFromColors(Settings::PoemTextFontColor));

    for (int col = 0; col < numOfCols; ++col) {
        for (int row = 0; row < numOfRows; ++row) {
            QTableWidgetItem* item = tableViewWidget->item(row, col);
            if (item) {
                QStringList verseData = item->data(Qt::UserRole).toString().split("|", QString::SkipEmptyParts);
                if (verseData.size() == 4 && verseData.at(0) == "VerseData=") {
                    if (verseData.at(2).toInt() == vorder) {
                        tableViewWidget->scrollToItem(item, QAbstractItemView::PositionAtCenter);
                        item->setTextColor(SaagharWidget::matchedTextColor);
                        text = item->text();
                    }
                    else {
                        item->setTextColor(poemColor);
                    }
                }
            }
        }
    }

    return text;
}

void SaagharWidget::parentCatClicked()
{
    parentCatButton = qobject_cast<QPushButton*>(sender());
    QStringList itemData = parentCatButton->objectName().split("=");
    if (itemData.size() != 2 || itemData.at(0) != "CatID") {
        return;
    }

    //using 'processClickedItem()'
    bool OK = false;
    int idData = itemData.at(1).toInt(&OK);
    bool noError = false;

    if (OK && SaagharWidget::ganjoorDataBase) {
        noError = true;
    }
    processClickedItem("CatID", idData, noError);

//  parentCatButtonData.remove("CATEGORY_ID=");
//
//  GanjoorCat category;
//  category.init(0, 0, "", 0, "");
//  bool OK = false;
//  int CatID = parentCatButtonData.toInt(&OK);

//  //qDebug() << "parentCatButtonData=" << parentCatButtonData << "CatID=" << CatID;

//  if (OK && ganjoorDataBase)
//      category = ganjoorDataBase->getCategory(CatID);

//  clearSaagharWidget();
//  showCategory(category);
}

void SaagharWidget::showHome()
{
    processClickedItem("CatID", 0, true);
//  GanjoorCat homeCat;
//  homeCat.init(0, 0, "", 0, "");
//  clearSaagharWidget();
//  showCategory(homeCat);
}

bool SaagharWidget::nextPoem()
{
    if (ganjoorDataBase->isConnected()) {
        GanjoorPoem poem = ganjoorDataBase->getNextPoem(currentPoem, currentCat);
        if (!poem.isNull()) {
            processClickedItem("PoemID", poem._ID, true);
//          clearSaagharWidget();
//          showPoem(poem);
            return true;
        }
    }
    return false;
}

bool SaagharWidget::previousPoem()
{
    if (ganjoorDataBase->isConnected()) {
        GanjoorPoem poem = ganjoorDataBase->getPreviousPoem(currentPoem, currentCat);
        if (!poem.isNull()) {
            processClickedItem("PoemID", poem._ID, true);
//          clearSaagharWidget();
//          showPoem(poem);
            return true;
        }
    }
    return false;
}

bool SaagharWidget::initializeCustomizedHome()
{
    if (SaagharWidget::maxPoetsPerGroup == 0) {
        return false;
    }

    QList<GanjoorPoet*> poets = SaagharWidget::ganjoorDataBase->getPoets();

    //tableViewWidget->clearContents();

    int numOfPoets = poets.size();
    int startIndex = 0;
    int numOfColumn, numOfRow;
    if (numOfPoets > SaagharWidget::maxPoetsPerGroup) {
        if (SaagharWidget::maxPoetsPerGroup == 1) {
            startIndex = 0;
        }
        else {
            startIndex = 1;
        }

        numOfRow = SaagharWidget::maxPoetsPerGroup + startIndex; //'startIndex=1' is for group title
        if ((numOfPoets / SaagharWidget::maxPoetsPerGroup)*SaagharWidget::maxPoetsPerGroup == numOfPoets) {
            numOfColumn = numOfPoets / SaagharWidget::maxPoetsPerGroup;
        }
        else {
            numOfColumn = 1 + numOfPoets / SaagharWidget::maxPoetsPerGroup;
        }
    }
    else {
        /*numOfColumn = 1;
        numOfRow = numOfPoets;*/
        return false;
    }
    tableViewWidget->setColumnCount(numOfColumn);
    tableViewWidget->setRowCount(numOfRow);

    QFont sectionFont(Settings::getFromFonts(Settings::SectionNameFontColor));
    //QFontMetrics sectionFontMetric(sectionFont);
    QColor sectionColor(Settings::getFromColors(Settings::SectionNameFontColor));

    int poetIndex = 0;
    for (int col = 0; col < numOfColumn; ++col) {
        QString groupLabel = "";
        for (int row = 0; row < SaagharWidget::maxPoetsPerGroup; ++row) {
            if (startIndex == 1) {
                if (row == 0) {
                    groupLabel = poets.at(poetIndex)->_Name;    //.at(0);
                }
                if (row == SaagharWidget::maxPoetsPerGroup - 1 || poetIndex == numOfPoets - 1) {
                    QString tmp = poets.at(poetIndex)->_Name;

                    if (groupLabel != tmp) {
                        int index = 0;
                        while (groupLabel.at(index) == tmp.at(index)) {
                            ++index;
                            if (index >= groupLabel.size() || index >= tmp.size()) {
                                --index;
                                break;
                            }
                        }
                        groupLabel = groupLabel.left(index + 1) + "-" + tmp.left(index + 1);
                    }
                    else {
                        groupLabel = groupLabel.at(0);
                    }


                    QTableWidgetItem* groupLabelItem = new QTableWidgetItem(groupLabel);
                    groupLabelItem->setFont(Settings::getFromFonts(Settings::TitlesFontColor));
                    groupLabelItem->setForeground(Settings::getFromColors(Settings::TitlesFontColor));
                    groupLabelItem->setFlags(Qt::NoItemFlags);
                    groupLabelItem->setTextAlignment(Qt::AlignCenter);
                    tableViewWidget->setItem(0, col, groupLabelItem);
                    if (col == 0) {
                        tableViewWidget->setRowHeight(0, SaagharWidget::computeRowHeight(QFontMetrics(Settings::getFromFonts(Settings::TitlesFontColor)), -1, -1));
                    }
                }
            }
            QTableWidgetItem* catItem = new QTableWidgetItem(poets.at(poetIndex)->_Name + "       ");
            catItem->setFont(sectionFont);
            catItem->setForeground(sectionColor);
            catItem->setFlags(catsItemFlag);
            catItem->setData(Qt::UserRole, "CatID=" + QString::number(poets.at(poetIndex)->_CatID));
            //poets.at(poetIndex)->_ID
            QString poetPhotoFileName = poetsImagesDir + "/" + QString::number(poets.at(poetIndex)->_ID) + ".png";;
            if (!QFile::exists(poetPhotoFileName)) {
                poetPhotoFileName = ICON_PATH + "/no-photo.png";
            }
            if (Settings::READ("Show Photo at Home").toBool()) {
                catItem->setIcon(QIcon(poetPhotoFileName));
            }
            else {
                catItem->setIcon(QIcon());
            }

            tableViewWidget->setItem(row + startIndex, col, catItem);
            ++poetIndex;
            if (poetIndex >= numOfPoets) {
                for (int i = row + 1; i < SaagharWidget::maxPoetsPerGroup; ++i) {
                    QTableWidgetItem* tmpItem = new QTableWidgetItem("");
                    tmpItem->setFlags(Qt::NoItemFlags);
                    tableViewWidget->setItem(i + startIndex, col, tmpItem);
                }
                break;
            }

            if (col == 0 && Settings::READ("Show Photo at Home").toBool()) {
                tableViewWidget->setRowHeight(row + startIndex, 105);
            }
        }
        if (poetIndex >= numOfPoets) {
            break;
        }
    }
    tableViewWidget->resizeColumnsToContents();
    //tableViewWidget->resizeRowsToContents();
    dirty = false;//page is showed or refreshed

    return true;
}

void SaagharWidget::homeResizeColsRows()
{
    int numOfCols = tableViewWidget->columnCount();
    int numOfRows = tableViewWidget->rowCount();
    int startIndex = 0;
    if (numOfCols > 1 && SaagharWidget::maxPoetsPerGroup != 1) {
        startIndex = 1;
    }

    bool showPhoto = Settings::READ("Show Photo at Home").toBool();
    QFontMetrics sectionFontMetric = QFontMetrics(Settings::getFromFonts(Settings::SectionNameFontColor));

    int height = SaagharWidget::computeRowHeight(sectionFontMetric, -1, -1);
    height = showPhoto ? qMax(height, 105) : height;

    for (int col = 0; col < numOfCols; ++col) {
        int colWidth = 0;
        for (int row = startIndex; row < numOfRows; ++row) {
            if (col == 0) {
                tableViewWidget->setRowHeight(row, height);
            }

            if (tableViewWidget->item(row, col)) {
                int w = sectionFontMetric.width(tableViewWidget->item(row, col)->text());
                if (w > colWidth) {
                    colWidth = w;
                }
            }
        }
        colWidth += showPhoto ? 82 : 0;
        tableViewWidget->setColumnWidth(col, colWidth + sectionFontMetric.width("888"));
    }
}

void SaagharWidget::showCategory(GanjoorCat category)
{
    if (category.isNull()) {
        //showHome();
        GanjoorCat homeCat;
        homeCat.init(0, 0, "", 0, "");
        clearSaagharWidget();
        showCategory(homeCat);
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    showParentCategory(category);
    currentPoem = 0;//from showParentCategory
    currentPoemTitle = "";

    pageMetaInfo.id = currentCat;
    pageMetaInfo.type = SaagharWidget::CategoryViewerPage;

    //new Caption
    currentCaption = (currentCat == 0) ? tr("Home") : ganjoorDataBase->getPoetForCat(currentCat)._Name;//for Tab Title
    if (currentCaption.isEmpty()) {
        currentCaption = currentLocationList.at(0);
    }

    emit captionChanged();
    QList<GanjoorCat*> subcats = ganjoorDataBase->getSubCategories(category._ID);

    int subcatsSize = subcats.size();

    bool poetForCathasDescription = false;
    if (category._ParentID == 0) {
        poetForCathasDescription = !ganjoorDataBase->getPoetForCat(category._ID)._Description.isEmpty();
    }
    if (subcatsSize == 1 && (category._ParentID != 0 || (category._ParentID == 0 && !poetForCathasDescription))) {
        GanjoorCat firstCat;
        firstCat.init(subcats.at(0)->_ID, subcats.at(0)->_PoetID, subcats.at(0)->_Text, subcats.at(0)->_ParentID,  subcats.at(0)->_Url);
        clearSaagharWidget();
        showCategory(firstCat);
        QApplication::restoreOverrideCursor();
        return;
    }

    QList<GanjoorPoem*> poems = ganjoorDataBase->getPoems(category._ID);

    if (poems.size() == 1 && subcatsSize == 0 && (category._ParentID != 0 || (category._ParentID == 0 && !poetForCathasDescription))) {
        GanjoorPoem firstPoem;
        firstPoem.init(poems.at(0)->_ID, poems.at(0)->_CatID, poems.at(0)->_Title, poems.at(0)->_Url,  poems.at(0)->_Faved,  poems.at(0)->_HighlightText);
        clearSaagharWidget();
        showPoem(firstPoem);
        QApplication::restoreOverrideCursor();
        return;
    }
    ///Initialize Table//TODO: I need to check! maybe it's not needed
    tableViewWidget->clearContents();

    tableViewWidget->setLayoutDirection(Qt::RightToLeft);

    int startRow = 0;
    if (currentCat == 0) {
        tableViewWidget->setIconSize(QSize(82, 100));
        bool customHome = true;
        if (customHome) {
            tableViewWidget->horizontalHeader()->setStretchLastSection(false);
            if (initializeCustomizedHome()) {
                QApplication::restoreOverrideCursor();
                return;
            }
            tableViewWidget->horizontalHeader()->setStretchLastSection(true);
        }
        tableViewWidget->setColumnCount(1);
        tableViewWidget->setRowCount(subcatsSize + poems.size());
    }
    else {
        tableViewWidget->setColumnCount(1);
        GanjoorPoet gPoet = SaagharWidget::ganjoorDataBase->getPoetForCat(category._ID);
        QString itemText = gPoet._Description;
        if (!itemText.isEmpty() && category._ParentID == 0) {
            startRow = 1;
            tableViewWidget->setRowCount(1 + subcatsSize + poems.size());
            QTableWidgetItem* catItem = new QTableWidgetItem(""/*itemText*/);
            catItem->setFlags(catsItemFlag);
            qDebug() << "showCategory";
            catItem->setTextAlignment(Qt::AlignJustify);
            catItem->setData(Qt::UserRole, "CatID=" + QString::number(category._ID));
            QString poetPhotoFileName = poetsImagesDir + "/" + QString::number(gPoet._ID) + ".png";;
            if (!QFile::exists(poetPhotoFileName)) {
                poetPhotoFileName = ICON_PATH + "/no-photo.png";
            }
            catItem->setIcon(QIcon(poetPhotoFileName));
            QTextEdit* descContainer = createItemForLongText(0, 0, itemText, SaagharWidget::lineEditSearchText->text());
            descContainer->setTextInteractionFlags(Qt::NoTextInteraction);
            descContainer->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            descContainer->setStyleSheet("\
                                         QTextEdit{border: transparent;\
                                                   background: transparent;\
                                         }\
                                         QScrollBar:vertical {\
                                             border: none;\
                                             background: transparent;\
                                             width: 13px;\
                                         }\
                                         QScrollBar::handle:vertical {\
                                             border: none;\
                                             background: #bebebe;\
                                             min-height: 20px;\
                                         }\
                                         QScrollBar::add-line:vertical {\
                                             height: 0px;\
                                         }\
                                         \
                                         QScrollBar::sub-line:vertical {\
                                             height: 0px;\
                                         }\
                                         \
                                         QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {\
                                             background: none;\
                                         }\
                                         \
                                         QScrollBar:vertical:hover {\
                                             border: 1px solid #bbb;\
                                             background: transparent;\
                                         }\
                                         QScrollBar::handle:vertical:hover {\
                                             background: #aaa;\
                                         }\
                                         ");
            descContainer->setMaximumHeight(200);
            /*if (category._ID == 0 && !Settings::READ("Show Photo at Home").toBool())
            {
                qDebug() << "Remove Icon11111111";
                catItem->setIcon(QIcon());
            }*/
            /*************
            //set row height
            int textWidth = tableViewWidget->fontMetrics().boundingRect(itemText).width();
            int verticalScrollBarWidth=0;
            if ( tableViewWidget->verticalScrollBar()->isVisible() )
            {
                verticalScrollBarWidth=tableViewWidget->verticalScrollBar()->width();
            }
            int totalWidth = tableViewWidget->columnWidth(0)-verticalScrollBarWidth-82;
            totalWidth = qMax(82+verticalScrollBarWidth, totalWidth);
            //int numOfRow = textWidth/totalWidth ;
            tableViewWidget->setRowHeight(0, qMax(100, SaagharWidget::computeRowHeight(tableViewWidget->fontMetrics(), textWidth, totalWidth)) );
            *************/
            //////tableViewWidget->setRowHeight(0, 2*tableViewWidget->rowHeight(0)+(tableViewWidget->fontMetrics().height()*(numOfRow/*+1*/)));
            //fixed bug: description's height
            resizeTable(tableViewWidget);
            tableViewWidget->setItem(0, 0, catItem);
        }
        else {
            tableViewWidget->setRowCount(subcatsSize + poems.size());
        }
    }

    QFont sectionFont(Settings::getFromFonts(Settings::SectionNameFontColor));
    //QFontMetrics sectionFontMetric(sectionFont);
    QColor sectionColor(Settings::getFromColors(Settings::SectionNameFontColor));

    int betterRightToLeft = 0, betterLeftToRight = 0;
    int step = 99;

    emit loadingStatusText(tr("<i><b>Loading the \"%1\"...</b></i>").arg(currentCaption), (poems.size() + subcatsSize) / (step + 1));

    for (int i = 0; i < subcatsSize; ++i) {
        QString catText = QGanjoorDbBrowser::simpleCleanString(subcats.at(i)->_Text);

        if (catText.isRightToLeft()) {
            ++betterRightToLeft;
        }
        else {
            ++betterLeftToRight;
        }

        QTableWidgetItem* catItem = new QTableWidgetItem(catText);
        catItem->setFont(sectionFont);
        catItem->setForeground(sectionColor);
        catItem->setFlags(catsItemFlag);
        catItem->setData(Qt::UserRole, "CatID=" + QString::number(subcats.at(i)->_ID));

        if (currentCat == 0) {
            GanjoorPoet gPoet = SaagharWidget::ganjoorDataBase->getPoetForCat(subcats.at(i)->_ID);
            QString poetPhotoFileName = poetsImagesDir + "/" + QString::number(gPoet._ID) + ".png";
            if (!QFile::exists(poetPhotoFileName)) {
                poetPhotoFileName = ICON_PATH + "/no-photo.png";
            }
            if (Settings::READ("Show Photo at Home").toBool()) {
                catItem->setIcon(QIcon(poetPhotoFileName));
            }
            else {
                catItem->setIcon(QIcon());
            }
        }
        tableViewWidget->setItem(i + startRow, 0, catItem);
        tableViewWidget->setRowHeight(i + startRow, SaagharWidget::computeRowHeight(QFontMetrics(sectionFont), -1, -1));

        //freeing resuorce
        delete subcats[i];
        subcats[i] = 0;

        if (i >= step) {
            emit loadingStatusText(tr("<i><b>Loading the \"%1\"...</b></i>").arg(currentCaption));
            step = step + 100;
        }
    }

    for (int i = 0; i < poems.size(); i++) {
        QString itemText = QGanjoorDbBrowser::snippedText(QGanjoorDbBrowser::simpleCleanString(poems.at(i)->_Title), "", 0, 15, true);
        if (subcatsSize > 0) {
            itemText.prepend("       ");    //7 spaces
        }
        itemText += " : " + QGanjoorDbBrowser::simpleCleanString(ganjoorDataBase->getFirstMesra(poems.at(i)->_ID));

        if (itemText.isRightToLeft()) {
            ++betterRightToLeft;
        }
        else {
            ++betterLeftToRight;
        }

        QTableWidgetItem* poemItem = new QTableWidgetItem(itemText);
        poemItem->setFont(sectionFont);
        poemItem->setForeground(sectionColor);
        poemItem->setFlags(poemsItemFlag);
        poemItem->setData(Qt::UserRole, "PoemID=" + QString::number(poems.at(i)->_ID));

        //we need delete all
        delete poems[i];
        poems[i] = 0;

        tableViewWidget->setItem(subcatsSize + i + startRow, 0, poemItem);
        tableViewWidget->setRowHeight(subcatsSize + i + startRow, SaagharWidget::computeRowHeight(QFontMetrics(sectionFont), -1, -1));

        if (subcatsSize + i > step) {
            emit loadingStatusText(tr("<i><b>Loading the \"%1\"...</b></i>").arg(currentCaption));
            step = step + 100;
        }
    }

    //support LTR contents
    if (betterRightToLeft >= betterLeftToRight) {
        tableViewWidget->setLayoutDirection(Qt::RightToLeft);
    }
    else {
        tableViewWidget->setLayoutDirection(Qt::LeftToRight);
    }

    applyDefaultSectionsHeight();

    if (!poems.isEmpty() || !subcats.isEmpty()) {
        resizeTable(tableViewWidget);
    }
    QApplication::restoreOverrideCursor();

    emit navPreviousActionState(!ganjoorDataBase->getPreviousPoem(currentPoem, currentCat).isNull());
    emit navNextActionState(!ganjoorDataBase->getNextPoem(currentPoem, currentCat).isNull());

    dirty = false;//page is showed or refreshed
}

void SaagharWidget::showParentCategory(GanjoorCat category)
{
    currentLocationList.clear();

    parentCatsToolBar->clear();
    parentCatsToolBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    parentCatsToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea | Qt::NoToolBarArea);
    //the parents of this category
    QList<GanjoorCat> ancestors = ganjoorDataBase->getParentCategories(category);

//  QHBoxLayout *parentCatLayout = new QHBoxLayout();
//  QWidget *parentCatWidget = new QWidget();
//  parentCatLayout->setSpacing(0);
//  parentCatLayout->setContentsMargins(0,0,0,0);

    for (int i = 0; i < ancestors.size(); i++) {
        QString buttonImage = ":/resources/cats-buttons/button-middle.png";
        QString buttonHomePressed, buttonHomeHovered;
        buttonHomePressed = buttonHomeHovered = buttonImage;

        parentCatButton = new QPushButton(parentCatsToolBar);
        int minWidth = parentCatButton->fontMetrics().width(ancestors.at(i)._Text) + 4;

        if (ancestors.size() == 1 && category._Text.isEmpty()) {
            buttonImage = ":/resources/cats-buttons/button-home-single.png";
            buttonHomeHovered = ":/resources/cats-buttons/button-home-single_hovered.png";
            buttonHomePressed = ":/resources/cats-buttons/button-home-single_pressed.png";
            //minWidth = 12;
            parentCatButton->setFixedWidth(32);
        }
        else {
            if (i == 0) {
                buttonImage = ":/resources/cats-buttons/button-home-start.png";
                buttonHomeHovered = ":/resources/cats-buttons/button-home-start_hovered.png";
                buttonHomePressed = ":/resources/cats-buttons/button-home-start_pressed.png";
                //minWidth = 22;
                parentCatButton->setFixedWidth(22);
            }
        }

        QString styleSheetStr = QString("QPushButton {\
                color: #606060;\
                min-height: 22px;\
                width: %1px;\
                padding-left: 15px;\
                border-image: url(%2) 0 15 0 15;\
                border-top: 0px transparent;\
                border-bottom: 0px transparent;\
                border-right: 15px transparent;\
                border-left: 15px transparent; }\
                QPushButton:hover { font: bold; color: black; border-image: url(%3) 0 15 0 15; }\
                QPushButton:pressed { color: grey; border-image: url(%4) 0 15 0 15; }\
                ").arg(minWidth).arg(buttonImage).arg(buttonHomeHovered).arg(buttonHomePressed);

        if (i == 0) {
            parentCatButton->setText("");
        }
        else {
            parentCatButton->setText(ancestors.at(i)._Text);
        }
        parentCatButton->setObjectName("CatID=" + QString::number(ancestors.at(i)._ID)); //used as button data
        connect(parentCatButton, SIGNAL(clicked(bool)), this, SLOT(parentCatClicked()));
        //style
        parentCatButton->setStyleSheet(styleSheetStr);
        //(QString("QPushButton{border: 2px solid #8f8f91; border-radius: 6px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 #dadbde); min-width: %1px; min-height: %2px; text margin-left:1px; margin-right:1px } QPushButton:pressed { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbde, stop: 1 #f6f7fa); } QPushButton:flat { border: none; } QPushButton:default { border-color: red; }").arg(minWidth).arg(parentCatButton->fontMetrics().height()+2));

        parentCatsToolBar->addWidget(parentCatButton);
//      parentCatLayout->addWidget(parentCatButton);

        if (i != 0) {
            currentLocationList << parentCatButton->text();
        }
    }

    if (!category._Text.isEmpty()) {
        parentCatButton = new QPushButton(parentCatsToolBar);
        parentCatButton->setText(category._Text);
        parentCatButton->setObjectName("CatID=" + QString::number(category._ID)); //used as button data
        connect(parentCatButton, SIGNAL(clicked(bool)), this, SLOT(parentCatClicked()));
        int minWidth = parentCatButton->fontMetrics().width(category._Text) + 6;
        QString styleSheetStr = QString("QPushButton {\
                color: #707070;\
                min-height: 22px;\
                min-width: %1px;\
                padding-left: 15px;\
                border-image: url(%2) 0 15 0 15;\
                border-top: 0px transparent;\
                border-bottom: 0px transparent;\
                border-right: 15px transparent;\
                border-left: 15px transparent; }\
                QPushButton:hover { color: black; }\
                QPushButton:pressed { color: grey; }\
                ").arg(minWidth).arg(":/resources/cats-buttons/button-last.png");

        parentCatButton->setStyleSheet(styleSheetStr);
        //QString("QPushButton{border: 2px solid #8f8f91; border-radius: 6px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 #dadbde); min-width: %1px; min-height: %2px; text margin-left:1px; margin-right:1px } QPushButton:pressed { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #dadbde, stop: 1 #f6f7fa); } QPushButton:flat { border: none; } QPushButton:default { border-color: red; }").arg(minWidth).arg(parentCatButton->fontMetrics().height()+2));
        parentCatsToolBar->addWidget(parentCatButton);
//      parentCatLayout->addWidget(parentCatButton);

        currentLocationList << parentCatButton->text();
    }
//  parentCatWidget->setLayout(parentCatLayout);
//  parentCatsToolBar->addWidget(parentCatWidget);
    parentCatsToolBar->setStyleSheet("QToolBar { background-image:url(\":/resources/images/transp.png\"); border:none; padding: 3px; spacing: 0px; /* spacing between items in the tool bar */ }");

    currentCat = !category.isNull() ? category._ID : 0;
    currentParentID = !category.isNull() ? category._ParentID : 0;
}
#include<QTime>
void SaagharWidget::showPoem(GanjoorPoem poem)
{
    if (poem.isNull()) {
        return;
    }

    minMesraWidth = 0;
    rowParagraphHeightMap.clear();
    rowSingleHeightMap.clear();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    showParentCategory(ganjoorDataBase->getCategory(poem._CatID));

    QList<GanjoorVerse*> verses = ganjoorDataBase->getVerses(poem._ID);

    QFont poemFont(Settings::getFromFonts(Settings::PoemTextFontColor));
    QFontMetrics poemFontMetric(poemFont);
    QColor poemColor(Settings::getFromColors(Settings::PoemTextFontColor));


    int WholeBeytNum = 0;
    int BeytNum = 0;
    int BandNum = 0;
    //Qt::Alignment groupedBeytAlignment = Qt::AlignLeft;

    tableViewWidget->setRowCount(2);
    tableViewWidget->setColumnCount(4);

    currentPoemTitle = poem._Title;

    //new Caption
    currentCaption = (currentCat == 0) ? tr("Home") : ganjoorDataBase->getPoetForCat(currentCat)._Name;//for Tab Title
    if (currentCaption.isEmpty()) {
        currentCaption = currentLocationList.at(0);
    }
    currentCaption += ":" + poem._Title;

    emit captionChanged();

    // disabling item at (0,0)
    QTableWidgetItem* flagItem = new QTableWidgetItem("");
    flagItem->setFlags(Qt::NoItemFlags);
    tableViewWidget->setItem(0, 0, flagItem);

    //Title of Poem
    QTableWidgetItem* poemTitle = new QTableWidgetItem(QGanjoorDbBrowser::simpleCleanString(poem._Title));
    poemTitle->setFlags(poemsTitleItemFlag);
    poemTitle->setData(Qt::UserRole, "PoemID=" + QString::number(poem._ID));

    //title is centered by using each PoemViewStyle
    poemTitle->setTextAlignment(Qt::AlignCenter);

    QFont titleFont(Settings::getFromFonts(Settings::TitlesFontColor));
    int textWidth = QFontMetrics(titleFont).boundingRect(poem._Title).width();
    int totalWidth = tableViewWidget->columnWidth(1) + tableViewWidget->columnWidth(2) + tableViewWidget->columnWidth(3);

    poemTitle->setFont(titleFont);
    poemTitle->setForeground(Settings::getFromColors(Settings::TitlesFontColor));
    tableViewWidget->setItem(0, 1, poemTitle);
    tableViewWidget->setSpan(0, 1, 1, 3);
    tableViewWidget->setRowHeight(0, SaagharWidget::computeRowHeight(QFontMetrics(titleFont), textWidth, totalWidth));
    rowSingleHeightMap.insert(0, textWidth);

    int row = 1;

    tableViewWidget->setLayoutDirection(Qt::RightToLeft);

//#ifndef Q_OS_MAC //Qt Bug when inserting TATWEEl character
    const bool justified = true;//temp
    int maxWidth = -1;
    if (longestHemistiches.contains(poem._ID)) {
        maxWidth = poemFontMetric.width(longestHemistiches.value(poem._ID));
    }
    int numberOfVerses = verses.size();
#ifdef SAAGHAR_DEBUG
    QTime start = QTime::currentTime();
#endif
    if (justified) {
        if (maxWidth <= 0) {
            QString longest = "";
            for (int i = 0; i < numberOfVerses; i++) {
                QString verseText = verses.at(i)->_Text;

                if (verses.at(i)->_Position == Single || verses.at(i)->_Position == Paragraph) {
                    continue;
                }

                verseText = verseText.simplified();
                int temp = poemFontMetric.width(verseText);
                if (temp > maxWidth) {
                    longest = verseText;
                    maxWidth = temp;
                }
            }
            longestHemistiches.insert(poem._ID, longest);
        }
#ifdef SAAGHAR_DEBUG
        qDebug() << "JustificationTime=" << -QTime::currentTime().msecsTo(start);
        start = QTime::currentTime();
#endif
    }
//#endif

    //temp and tricky way for some database problems!!(second Mesra when there is no a defined first Mesra)
    bool rightVerseFlag = false;
    int step = 99;
    emit loadingStatusText(tr("<i><b>Loading the \"%1\"...</b></i>").arg(QGanjoorDbBrowser::snippedText(poem._Title, "", 0, 6, false, Qt::ElideRight)), numberOfVerses / (step + 1));

    QStringList bookmarkedVerses("");
    if (SaagharWidget::bookmarks) {
        bookmarkedVerses = SaagharWidget::bookmarks->bookmarkList("Verses");
    }

    int betterRightToLeft = 0, betterLeftToRight = 0;
    //very Big For loop
    for (int i = 0; i < numberOfVerses; i++) {
        QString currentVerseText = verses.at(i)->_Text;
        if (verses.at(i)->_Position != Single && verses.at(i)->_Position != Paragraph) {
            currentVerseText = currentVerseText.simplified();
        }

        if (i >= step) {
            emit loadingStatusText(tr("<i><b>Loading the \"%1\"...</b></i>").arg(QGanjoorDbBrowser::snippedText(poem._Title, "", 0, 6, false, Qt::ElideRight)));
            step = step + 100;
        }
//#ifndef Q_OS_MAC //Qt Bug when inserting TATWEEl character
        if (justified) {
            currentVerseText = QGanjoorDbBrowser::justifiedText(currentVerseText, poemFontMetric, maxWidth);
        }
//#endif

        if (currentVerseText.isEmpty()) {
            if (verses.at(i)->_Position == Paragraph
                    || verses.at(i)->_Position == CenteredVerse1
                    || verses.at(i)->_Position == CenteredVerse2
                    || verses.at(i)->_Position == Single) {
                if (i == verses.size() - 1) {
                    tableViewWidget->removeRow(row);
                }
                continue;
            }

            if (verses.at(i)->_Position == Left) {
                bool empty = true;
                for (int k = 0; k < tableViewWidget->columnCount(); ++k) {
                    QTableWidgetItem* temp = tableViewWidget->item(row, k);
                    if (temp && temp->text().isEmpty()) {
                        empty = false;
                    }
                }

                if (empty) {
                    if (i == verses.size() - 1) {
                        tableViewWidget->removeRow(row);
                    }
                    continue;
                }
            }
        }

        if (currentVerseText.isRightToLeft()) {
            ++betterRightToLeft;
        }
        else {
            ++betterLeftToRight;
        }

        QTableWidgetItem* mesraItem = new QTableWidgetItem(currentVerseText);
        mesraItem->setFlags(versesItemFlag);
        //set data for mesraItem
        QString verseData = QString::number(verses.at(i)->_PoemID) + "|" + QString::number(verses.at(i)->_Order) + "|" + QString::number((int)verses.at(i)->_Position);
        mesraItem->setData(Qt::UserRole, "VerseData=|" + verseData);

        VersePosition versePosition = verses.at(i)->_Position;
        //temp and tricky way for some database problems!!(second Mesra when there is no a defined first Mesra)
        if (!rightVerseFlag && versePosition == Left) {
            versePosition = Paragraph;
        }
        //qDebug()<<"bedoL-vPos="<<versePosition;
        mesraItem->setFont(poemFont);
        mesraItem->setForeground(poemColor);

        doPoemLayout(&row, mesraItem, currentVerseText, poemFontMetric, versePosition/*, groupedBeytAlignment*/);
        //qDebug()<<"afterdoLay-vPos="<<versePosition;

        //temp and tricky way for some database problems!!(second Mesra when there is no a defined first Mesra)
        if (/*CurrentViewStyle == BeytPerLine &&*/ versePosition == Right) {
            rightVerseFlag = true;
        }

        QString simplifiedText = currentVerseText;
        simplifiedText.remove(" ");
        simplifiedText.remove("\t");
        simplifiedText.remove("\n");

        QString currentVerseData = QString::number(verses.at(i)->_PoemID) + "|" + QString::number(verses.at(i)->_Order);
        bool verseIsBookmarked = false;
        if (bookmarkedVerses.contains(currentVerseData)) {
            verseIsBookmarked = true;
        }

        if (!simplifiedText.isEmpty() && ((verses.at(i)->_Position == Single && !currentVerseText.isEmpty()) ||
                                          verses.at(i)->_Position == Right ||
                                          verses.at(i)->_Position == CenteredVerse1)
           ) {
            WholeBeytNum++;
            bool isBand = (verses.at(i)->_Position == CenteredVerse1);
            if (isBand) {
                BeytNum = 0;
                BandNum++;
                //groupedBeytAlignment = Qt::AlignLeft;
            }
            else {
                BeytNum++;
            }

            if (!currentVerseText.isEmpty()) {
                //empty verse strings have been seen sometimes, it seems that we have some errors in our database
                //QString verseData = QString::number(verses.at(i)->_PoemID)+"."+QString::number(verses.at(i)->_Order)+"."+QString::number((int)verses.at(i)->_Position);
                QTableWidgetItem* numItem = new QTableWidgetItem("");

                if (showBeytNumbers) {
                    int itemNumber = isBand ? BandNum : BeytNum;
                    QString localizedNumber = SaagharWidget::persianIranLocal.toString(itemNumber);
                    numItem->setText(localizedNumber);
                    numItem->setFont(Settings::getFromFonts(Settings::NumbersFontColor));
                    numItem->setForeground(Settings::getFromColors(Settings::NumbersFontColor));
                    if (isBand) {
                        QFont fnt = numItem->font();
                        fnt.setBold(true);
                        fnt.setItalic(true);
                        numItem->setFont(fnt);
                        numItem->setForeground(QBrush(QColor(Qt::yellow).darker(150)));
                    }
                }
                numItem->setFlags(numItemFlags);
                if (SaagharWidget::bookmarks) {
                    QPixmap star(ICON_PATH + "/bookmark-on.png");
                    QPixmap starOff(ICON_PATH + "/bookmark-off.png");
                    star = star.scaledToHeight(qMin(tableViewWidget->rowHeight(row) - 1, 22), Qt::SmoothTransformation);
                    starOff = starOff.scaledToHeight(qMin(tableViewWidget->rowHeight(row) - 1, 22), Qt::SmoothTransformation);
                    QIcon bookmarkIcon;
                    bookmarkIcon.addPixmap(star, QIcon::Active, QIcon::On);
                    bookmarkIcon.addPixmap(starOff, QIcon::Disabled, QIcon::Off);
                    numItem->setData(ITEM_BOOKMARKED_STATE, verseIsBookmarked);
                    if (verseIsBookmarked) {
                        bookmarkIcon = bookmarkIcon.pixmap(star.size(), QIcon::Active, QIcon::On);
                    }
                    else {
                        bookmarkIcon = bookmarkIcon.pixmap(star.size(), QIcon::Disabled, QIcon::Off);
                    }
                    numItem->setIcon(bookmarkIcon);
                }
                tableViewWidget->setItem(row, 0, numItem);
            }
        }

        if (verses.at(i)->_Position == Paragraph ||
                verses.at(i)->_Position == Left ||
                verses.at(i)->_Position == CenteredVerse1 ||
                verses.at(i)->_Position == CenteredVerse2 ||
                verses.at(i)->_Position == Single) {
            QTableWidgetItem* numItem = tableViewWidget->item(row, 0);
            if (!numItem) {
                numItem = new QTableWidgetItem("");
                numItem->setFlags(numItemFlags);
                if (SaagharWidget::bookmarks && verses.at(i)->_Position == Paragraph) {
                    QPixmap star(ICON_PATH + "/bookmark-on.png");
                    QPixmap starOff(ICON_PATH + "/bookmark-off.png");
                    star = star.scaledToHeight(qMin(tableViewWidget->rowHeight(row) - 1, 22), Qt::SmoothTransformation);
                    starOff = starOff.scaledToHeight(qMin(tableViewWidget->rowHeight(row) - 1, 22), Qt::SmoothTransformation);
                    QIcon bookmarkIcon;
                    bookmarkIcon.addPixmap(star, QIcon::Active, QIcon::On);
                    bookmarkIcon.addPixmap(starOff, QIcon::Disabled, QIcon::Off);
                    numItem->setData(ITEM_BOOKMARKED_STATE, verseIsBookmarked);
                    if (verseIsBookmarked) {
                        bookmarkIcon = bookmarkIcon.pixmap(star.size(), QIcon::Active, QIcon::On);
                    }
                    else {
                        bookmarkIcon = bookmarkIcon.pixmap(star.size(), QIcon::Disabled, QIcon::Off);
                    }
                    numItem->setIcon(bookmarkIcon);
                }
                tableViewWidget->setItem(row, 0, numItem);
            }
            rightVerseFlag = false; //temp and tricky way for some database problems!!(second Mesra when there is no a defined first Mesra)
            ++row;

//          if (verses.at(i)->_Position == Left && !currentVerseText.isEmpty())
//              groupedBeytAlignment = (groupedBeytAlignment == Qt::AlignRight ? Qt::AlignLeft : Qt::AlignRight);
//          else
//              groupedBeytAlignment = Qt::AlignLeft;

            if (i != verses.size() - 1) {
                tableViewWidget->insertRow(row);
            }
        }

        //delete Verses
        delete verses[i];
        verses[i] = 0;
    }// end of big for

    //support LTR contents
    if (betterRightToLeft >= betterLeftToRight) {
        tableViewWidget->setLayoutDirection(Qt::RightToLeft);
    }
    else {
        tableViewWidget->setLayoutDirection(Qt::LeftToRight);
    }
#ifdef SAAGHAR_DEBUG
    qDebug() << "end of poem layouting=" << -QTime::currentTime().msecsTo(start);
#endif

    //a trick for removing last empty row withoout QTextEdit widget
    if (!tableViewWidget->cellWidget(tableViewWidget->rowCount() - 1, 1)) {
        QString rowStr = "";
        for (int col = 0; col < tableViewWidget->columnCount(); ++col) {
            QTableWidgetItem* tmpItem = tableViewWidget->item(tableViewWidget->rowCount() - 1, col);
            if (tmpItem) {
                rowStr += tmpItem->text();
            }
        }
        rowStr.remove(" ");
        rowStr.remove("\t");
        rowStr.remove("\n");
        if (rowStr.isEmpty()) {
            tableViewWidget->setRowCount(tableViewWidget->rowCount() - 1);
        }
    }

    currentPoem = poem._ID;

    applyDefaultSectionsHeight();
    resizeTable(tableViewWidget);

    QApplication::restoreOverrideCursor();

#ifndef NO_PHONON_LIB
    if (SaagharWidget::musicPlayer) {
        pageMetaInfo.id = currentPoem;
        pageMetaInfo.type = SaagharWidget::PoemViewerPage;

        if (SaagharWidget::musicPlayer->albumContains(currentPoem, 0)) {
            QString path;
            QString title;
            SaagharWidget::musicPlayer->getFromAlbum(currentPoem, &path, &title);
            if (SaagharWidget::musicPlayer->source() != path) {
                SaagharWidget::musicPlayer->setSource(path, currentLocationList.join(">") + ">" + currentPoemTitle, currentPoem);
            }
        }
        else {
            //at startup we load everythings!
            SaagharWidget::musicPlayer->setSource("", currentLocationList.join(">") + ">" + currentPoemTitle, currentPoem);
        }
    }
#endif //NO_PHONON_LIB

    emit navPreviousActionState(!ganjoorDataBase->getPreviousPoem(currentPoem, currentCat).isNull());
    emit navNextActionState(!ganjoorDataBase->getNextPoem(currentPoem, currentCat).isNull());

    dirty = false;//page is showed or refreshed
}

void SaagharWidget::clearSaagharWidget()
{
    lastOveredItem = 0;
    tableViewWidget->setRowCount(0);
    tableViewWidget->setColumnCount(0);
}

QString SaagharWidget::currentPageGanjoorUrl()
{
    if (currentCat == 0) {
        return "http://ganjoor.net";
    }
    if (currentPoem == 0) {
        return ganjoorDataBase->getCategory(currentCat)._Url;
    }
    return ganjoorDataBase->getPoem(currentPoem)._Url;
    /*
     * using following code you can delete url field in poem table,
     *
     * return "http://ganjoor.net/?p=" + currentPoem.ToString();
     *
     * it causes ganjoor.net to perform a redirect to SEO frinedly url,
     * however size of database is reduced only by 2 mb (for 69 mb one) in this way
     * so I thought it might not condisered harmful to keep current structure.
     */
}

void SaagharWidget::resizeTable(QTableWidget* table)
{
    if (table && table->columnCount() > 0) {
        if (currentCat == 0  && currentPoem == 0) { //it's Home.
            homeResizeColsRows();
            return;
        }

        QString vV = "False";
        int verticalScrollBarWidth = 0;
        if (table->verticalScrollBar()->isVisible()) {
            vV = "true";
            verticalScrollBarWidth = table->verticalScrollBar()->width();
        }
        int baseWidthSize = parentWidget()->width() - (2 * table->x() + verticalScrollBarWidth);

        //****************************//
        //Start colWidths computations//
        //****************************//
        QFontMetrics poemFontMetrics(Settings::getFromFonts(Settings::PoemTextFontColor));

        int iconWidth = 0;
        if (SaagharWidget::bookmarks) {
#ifdef Q_OS_MAC
            iconWidth = 35;
#else
            iconWidth = 22;
#endif
        }
        int test = 0;
        switch (table->columnCount()) {
        case 1:
            table->setColumnWidth(0, qMax(minMesraWidth, baseWidthSize)); //single mesra
            break;
        case 2:
            table->setColumnWidth(0, (baseWidthSize * 5) / 100);
            table->setColumnWidth(1, (baseWidthSize * 94) / 100);
            break;
        case 3:
            table->setColumnWidth(0, (baseWidthSize * 6) / 100);
            table->setColumnWidth(1, (baseWidthSize * 47) / 100);
            table->setColumnWidth(2, (baseWidthSize * 47) / 100);
            break;
        case 4:
            if (CurrentViewStyle == SteppedHemistichLine /*|| CurrentViewStyle==MesraPerLineGroupedBeyt*/) {
                table->setColumnWidth(0, SaagharWidget::showBeytNumbers ? QFontMetrics(Settings::getFromFonts(Settings::NumbersFontColor)).width(QString::number(table->rowCount() * 100)) + iconWidth : iconWidth + 3); //numbers
                baseWidthSize = baseWidthSize - table->columnWidth(0);
                table->setColumnWidth(2, qMax(qMin((7 * minMesraWidth) / 4, (7 * baseWidthSize) / 8), minMesraWidth)); // cells contain mesras
                test = qMax(0, baseWidthSize - (table->columnWidth(2)));
                table->setColumnWidth(1, test / 2); //right margin
                table->setColumnWidth(3, test / 2); //left margin
            }
            else if (CurrentViewStyle == OneHemistichLine) {
                table->setColumnWidth(0, SaagharWidget::showBeytNumbers ? QFontMetrics(Settings::getFromFonts(Settings::NumbersFontColor)).width(QString::number(table->rowCount() * 100)) + iconWidth : iconWidth + 3); //numbers
                baseWidthSize = baseWidthSize - table->columnWidth(0);
                table->setColumnWidth(2, qMax(0, minMesraWidth));  // cells contain mesras
                test = qMax(0, baseWidthSize - (table->columnWidth(2)));
                table->setColumnWidth(1, test / 2); //right margin
                table->setColumnWidth(3, test / 2); //left margin
            }
            else {
                table->setColumnWidth(0, SaagharWidget::showBeytNumbers ? QFontMetrics(Settings::getFromFonts(Settings::NumbersFontColor)).width(QString::number(table->rowCount() * 100)) + iconWidth : iconWidth + 3); //numbers
                int tw = baseWidthSize - (table->columnWidth(0) + poemFontMetrics.height() * 2/*table->columnWidth(2)*/);
                table->setColumnWidth(1, qMax(minMesraWidth, tw / 2/* -table->columnWidth(0) */)); //mesra width
                table->setColumnWidth(3, qMax(minMesraWidth, tw / 2)); //mesra width
                table->setColumnWidth(2, qMax(poemFontMetrics.height() + 1, baseWidthSize - (table->columnWidth(0) + table->columnWidth(1) + table->columnWidth(3)))); //spacing between mesras
            }
            break;
        default:
            break;
        }
        //**************************//
        //End colWidths computations//
        //**************************//


        //*****************************//
        //Start rowHeights computations//
        //*****************************//
        //resize description's row
        if (currentCat != 0  && currentPoem == 0 && table->columnCount() == 1) {
            //using column count here is a tricky way
            if (currentParentID == 0) {
                QString itemText;
                QTextEdit* textEdit = qobject_cast<QTextEdit*>(table->cellWidget(0, 0));
                if (textEdit) {
                    itemText = textEdit->toPlainText();
                }
                else {
                    GanjoorPoet gPoet = SaagharWidget::ganjoorDataBase->getPoetForCat(currentCat);
                    itemText = gPoet._Description;
                }

                if (!itemText.isEmpty()) {
                    int textWidth = QFontMetrics(Settings::getFromFonts(Settings::ProseTextFontColor)).boundingRect(itemText).width();
                    int verticalScrollBarWidth = 0;
                    if (table->verticalScrollBar()->isVisible()) {
                        verticalScrollBarWidth = table->verticalScrollBar()->width();
                    }
                    int totalWidth = table->columnWidth(0) - verticalScrollBarWidth - 82;
                    totalWidth = qMax(82 + verticalScrollBarWidth, totalWidth);
                    table->setRowHeight(0, qMin(200, qMax(100, SaagharWidget::computeRowHeight(QFontMetrics(Settings::getFromFonts(Settings::ProseTextFontColor)), textWidth, totalWidth))));
                }
            }
        }

        //resize rows that contains 'Paragraph' and 'Single'
        int totalWidth = 0;
        if (table->columnCount() == 4) {
            totalWidth = table->columnWidth(1) + table->columnWidth(2) + table->columnWidth(3);

            QMap<int, QPair<int, int> >::const_iterator i = rowParagraphHeightMap.constBegin();
            while (i != rowParagraphHeightMap.constEnd()) {
                //Paragraphs
                QFontMetrics paragraphFontMetric(Settings::getFromFonts(Settings::ProseTextFontColor));
                int height = SaagharWidget::computeRowHeight(paragraphFontMetric, i.value().first, totalWidth , (5 * paragraphFontMetric.height()) / 3);
                height = height + i.value().second * paragraphFontMetric.height();
                table->setRowHeight(i.key(), height);
                ++i;
            }

            QMap<int, int>::const_iterator it = rowSingleHeightMap.constBegin();
            while (it != rowSingleHeightMap.constEnd()) {
                //Singles and Titles
                if (currentPoem && it.key() == 0) {
                    table->setRowHeight(0, SaagharWidget::computeRowHeight(QFontMetrics(Settings::getFromFonts(Settings::TitlesFontColor)), it.value(), totalWidth /*, table->rowHeight(i.key())*/));
                }
                else {
                    int height = qMax(SaagharWidget::computeRowHeight(QFontMetrics(Settings::getFromFonts(Settings::PoemTextFontColor)), it.value(), totalWidth), iconWidth * 5 / 4);
                    if (SaagharWidget::showBeytNumbers) {
                        height = qMax(height, SaagharWidget::computeRowHeight(QFontMetrics(Settings::getFromFonts(Settings::NumbersFontColor)), -1, -1));
                    }
                    table->setRowHeight(it.key(), height);
                }

                ++it;
            }
        }
        //***************************//
        //End rowHeights computations//
        //***************************//
    }
}

QTableWidgetItem* SaagharWidget::scrollToFirstItemContains(const QString &phrase, bool pharseIsList, bool scroll)
{
    QString keyword = phrase;
    keyword.replace(QChar(0x200C), "", Qt::CaseInsensitive);//replace ZWNJ by ""
    keyword.replace(QChar(0x0640), "", Qt::CaseInsensitive);//replace TATWEEL by ""

    if (!pharseIsList) { //TODO: maybe when using list mode this create a bug
        keyword = QGanjoorDbBrowser::cleanString(keyword);
    }

    if (keyword.isEmpty()) {
        return 0;
    }

    QStringList list(keyword);
    if (pharseIsList) {
        list = SearchPatternManager::phraseToList(keyword, false);
    }

    if (list.isEmpty()) {
        return 0;
    }
    int listSize = list.size();

    for (int row = 1; row < tableViewWidget->rowCount(); ++row) {
        //start from second row, we need to skip poem's title.
        for (int col = 0; col < tableViewWidget->columnCount(); ++col) {
            QTableWidgetItem* tmp = tableViewWidget->item(row, col);
            if (tmp) {
                QString text = tmp->text();
                if (text.isEmpty()) {
                    QTextEdit* textEdit = qobject_cast<QTextEdit*>(tableViewWidget->cellWidget(row, col));
                    if (textEdit) {
                        text = textEdit->toPlainText();
                    }
                }
                text =  QGanjoorDbBrowser::cleanString(text);
                text.remove(".");//remove because of elided text
                text.replace(QChar(0x0640), "", Qt::CaseInsensitive);//replace TATWEEL by ""
                text = text.simplified();
                if (text.isEmpty()) {
                    continue;
                }

                for (int i = 0; i < listSize; ++i) {
                    keyword = list.at(listSize - 1 - i); //start from last, probably it's the new one!
                    keyword.remove(".");//remove because of elided text

                    if (keyword.contains("@")) {
                        keyword.replace("@", "\\S*", Qt::CaseInsensitive);//replace wildcard by word chars
                        QRegExp regExp(keyword, Qt::CaseInsensitive);
                        regExp.indexIn(text);
                        keyword = regExp.cap(0);
                        if (keyword.isEmpty()) {
                            continue;
                        }
                    }
                    keyword = keyword.simplified();

                    if (text.contains(keyword)) {
                        if (scroll) {
                            //TODO: there is a BUG! (search:دختر Sadi, حکایت 42 or 43!)
                            tableViewWidget->setCurrentItem(tmp, QItemSelectionModel::NoUpdate);
                            tableViewWidget->scrollToItem(tmp, QAbstractItemView::PositionAtCenter);
                        }
                        //row = tableViewWidget->rowCount()+1;
                        //col = tableViewWidget->columnCount()+1;
                        //break;
                        return tmp;
                    }
                }
            }
        }
    }
    return 0;
}

int SaagharWidget::computeRowHeight(const QFontMetrics &fontMetric, int textWidth, int width, int height)
{
    if (width <= 0 || textWidth <= 0) {
        return 4 * (fontMetric.height() / 3);
    }
    if (height == 0) {
        height = (4 * fontMetric.height()) / 3;
    }
    int numOfRow = (textWidth * 5) / (width * 4); //(5/4) is an empirical value
    return height + (fontMetric.height() * numOfRow);
}

void SaagharWidget::pressedOnItem(int row, int /*col*/)
{
    pressedPosition = QCursor::pos();
    clickedOnItem(row, -1);
}

void SaagharWidget::setFromMVPosition()
{
    if (m_vPosition > 0) {
        setVerticalPosition(m_vPosition);
        m_vPosition = -1;
    }
}

void SaagharWidget::clickedOnItem(int row, int column)
{
    if (column == 0) {
        QTableWidgetItem* item = tableViewWidget->item(row, 0);
        QTableWidgetItem* verseItem = 0;
        if (tableViewWidget->columnCount() > 1) {
            if (SaagharWidget::CurrentViewStyle == TwoHemistichLine ||
                    SaagharWidget::CurrentViewStyle == OneHemistichLine) {
                verseItem = tableViewWidget->item(row, 1);
            }
            else if (SaagharWidget::CurrentViewStyle == SteppedHemistichLine) {
                verseItem = tableViewWidget->item(row, 2);
                if (!verseItem) {
                    //it's Paragraph, Single, CenteredVerse1/2 or empty!
                    verseItem = tableViewWidget->item(row, 1);
                }
            }
        }

        if (verseItem && item && !item->icon().isNull()) {
            QStringList data = verseItem->data(Qt::UserRole).toString().split("|");
            if (data.size() != 4) {
                return;
            }

            QPixmap star(ICON_PATH + "/bookmark-on.png");
            QPixmap starOff(ICON_PATH + "/bookmark-off.png");
            star = star.scaledToHeight(qMin(tableViewWidget->rowHeight(row) - 1, 22), Qt::SmoothTransformation);
            starOff = starOff.scaledToHeight(qMin(tableViewWidget->rowHeight(row) - 1, 22), Qt::SmoothTransformation);
            QIcon bookmarkIcon;
            bookmarkIcon.addPixmap(star, QIcon::Active, QIcon::On);
            bookmarkIcon.addPixmap(starOff, QIcon::Disabled, QIcon::Off);
            bool bookmarkState = !item->data(ITEM_BOOKMARKED_STATE).toBool();

            if (bookmarkState) {
                bookmarkIcon = bookmarkIcon.pixmap(star.size(), QIcon::Active, QIcon::On);
            }
            else {
                bookmarkIcon = bookmarkIcon.pixmap(star.size(), QIcon::Disabled, QIcon::Off);
            }

            QString verseText = verseItem->text().simplified();
            if (verseText.isEmpty()) {
                //maybe cell has QTextEdit as its widget!
                QTextEdit* textEdit = qobject_cast<QTextEdit*>(tableViewWidget->cellWidget(row, 1));
                if (textEdit) {
                    verseText = textEdit->toPlainText().simplified();
                    verseText = QGanjoorDbBrowser::snippedText(verseText, "", 0, 20);
                }
            }

            QString currentLocation = currentLocationList.join(">");
            if (!currentPoemTitle.isEmpty()) {
                currentLocation += ">" + currentPoemTitle;
            }

            verseText.prepend(currentLocation + "\n");

            if (data.at(3).toInt() == Right && tableViewWidget->item(row + 1, 2) &&
                    (SaagharWidget::CurrentViewStyle == SteppedHemistichLine)) {
                verseText += "\n" + tableViewWidget->item(row + 1, 2)->text().simplified();
            }
            else if (tableViewWidget->columnCount() == 4 && tableViewWidget->item(row, 3)) {
                verseText += "\n" + tableViewWidget->item(row, 3)->text().simplified();
            }
            else if (data.at(3).toInt() == CenteredVerse1 && tableViewWidget->item(row + 1, 1)) {
                QStringList tmp = tableViewWidget->item(row + 1, 1)->data(Qt::UserRole).toString().split("|");

                if (tmp.size() == 4 && tmp.at(3).toInt() == CenteredVerse2) {
                    verseText += "\n" + tableViewWidget->item(row + 1, 1)->text().simplified();
                }
            }

            QChar tatweel = QChar(0x0640);
            verseText.remove(tatweel);

            if (SaagharWidget::bookmarks &&
                    SaagharWidget::bookmarks->updateBookmarkState("Verses", QStringList() << data.at(1) << data.at(2) << verseText/*+"\n"*/ << currentPageGanjoorUrl() + "/#" + data.at(2), bookmarkState)
               ) {
                item->setIcon(bookmarkIcon);
                item->setData(ITEM_BOOKMARKED_STATE, bookmarkState);
            }
        }
    }

    if (pressedPosition != QCursor::pos()) {
        pressedPosition = QPoint(-1, -1);
        return;
    }
    Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
    if ((modifiers & Qt::ShiftModifier) || (modifiers & Qt::ControlModifier)) {
        return;
    }

    QList<QTableWidgetItem*> selectedList = tableViewWidget->selectedItems();
    bool otherSelections = false;
    for (int i = 0; i < selectedList.size(); ++i) {
        if (selectedList.at(i) && selectedList.at(i)->row() != row) {
            otherSelections = true;
            break;
        }
    }

    if (otherSelections) {
        return;
    }

    for (int col = 0; col < tableViewWidget->columnCount(); ++col) {
        QTableWidgetItem* item = tableViewWidget->item(row, col);
        if (item && QApplication::mouseButtons() != Qt::RightButton) {
            if (item->isSelected()) {
                item->setSelected(false);
            }
        }
    }
}

QStringList SaagharWidget::identifier()
{
    QStringList tabViewType;
    if (currentPoem > 0) {
        tabViewType << "PoemID" << QString::number(currentPoem);
    }
    else {
        tabViewType << "CatID" << QString::number(currentCat);
    }

    return tabViewType;
}

void SaagharWidget::refresh()
{
    if (currentPoem > 0) {
        processClickedItem("PoemID", currentPoem, true);
    }
    else {
        processClickedItem("CatID", currentCat, true);
    }
}

QTextEdit* SaagharWidget::createItemForLongText(int row, int column, const QString &text, const QString &highlightText)
{
    if (!tableViewWidget) {
        return 0;
    }
    QTextEdit* para = new QTextEdit(tableViewWidget);
    para->setStyleSheet(QString("QTextEdit{background: transparent; border: none; selection-color: %1; selection-background-color: %2;}").arg(tableViewWidget->palette().color(QPalette::HighlightedText).name()).arg(tableViewWidget->palette().color(QPalette::Highlight).name()));
    //para->setAlignment(Qt::AlignJustify);
    para->setContextMenuPolicy(Qt::CustomContextMenu);
    //connect(para, SIGNAL(customContextMenuRequested(QPoint)), this, SIGNAL(createContextMenuRequested(QPoint)));
    connect(para, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(createCustomContextMenu(QPoint)));

    para->setFont(Settings::getFromFonts(Settings::ProseTextFontColor));
    para->setTextColor(Settings::getFromColors(Settings::ProseTextFontColor));
//  para->setFont(SaagharWidget::tableFont);
//  para->setTextColor(SaagharWidget::textColor);
    para->setReadOnly(true);
    para->setLineWrapMode(QTextEdit::WidgetWidth);
    para->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    para->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    para->setTextInteractionFlags(Qt::TextBrowserInteraction/*Qt::NoTextInteraction*/);
    para->setText(text);
    para->setLayoutDirection(text.isRightToLeft() ? Qt::RightToLeft : Qt::LeftToRight);
    QTextCursor tc = para->textCursor();
    tc.select(QTextCursor::Document);
    QTextBlockFormat tBF = tc.blockFormat();
    tBF.setAlignment(Qt::AlignJustify);
    tc.setBlockFormat(tBF);
    tc.clearSelection();
    para->setTextCursor(tc);
    //para->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    //para->setAttribute(Qt::WA_NoMousePropagation, true);
    //para->setMouseTracking(false);
    para->setFocusPolicy(Qt::NoFocus);
    ParagraphHighlighter* paraHighlighter = new ParagraphHighlighter(para->document(), highlightText);
    connect(SaagharWidget::lineEditSearchText, SIGNAL(textChanged(QString)), paraHighlighter, SLOT(keywordChanged(QString)));
    tableViewWidget->setCellWidget(row, column, para);
    return para;
}

void SaagharWidget::doPoemLayout(int* prow, QTableWidgetItem* mesraItem, const QString &currentVerseText, const QFontMetrics &fontMetric, VersePosition versePosition/*, Qt::Alignment beytAlignment*/)
{
    if (!mesraItem || !prow) {
        return;
    }
    int row = *prow;

    int BandBeytNums = 0;
    bool MustHave2ndBandBeyt = false;

    bool spacerColumnIsPresent = false;
    int firstEmptyThirdColumn = 1;
    bool flagEmptyThirdColumn = false;

    // mesra width computation: QChar(126, 6) is Pe
    const QString &extendString = QString(QChar(126, 6)) + QChar(126, 6);
    int textWidth;

    //Single and Paragraph are layouted just at one form!
    if (versePosition == Single) {
        textWidth = fontMetric.boundingRect(mesraItem->text()).width();

        if (!currentVerseText.isEmpty()) {
            tableViewWidget->setItem(row, 1, mesraItem);
            tableViewWidget->setSpan(row, 1, 1, 3);
        }

        rowSingleHeightMap.insert(row, textWidth);
        return;
    }
    else if (versePosition == Paragraph) {
        textWidth = QFontMetrics(Settings::getFromFonts(Settings::ProseTextFontColor)).boundingRect(mesraItem->text()).width();

        mesraItem->setText("");
        //inserted just for its data and its behavior like other cells that use QTableWidgetItem'.
        tableViewWidget->setItem(row, 1, mesraItem);
        createItemForLongText(row, 1, currentVerseText, SaagharWidget::lineEditSearchText->text());
        tableViewWidget->setSpan(row, 1, 1, 3);

        rowParagraphHeightMap.insert(row, QPair<int, int>(textWidth, currentVerseText.count("\n")));

        return;
    }

    switch (CurrentViewStyle) {
    case TwoHemistichLine:
        switch (versePosition) {
        case Right :
            spacerColumnIsPresent = true;
            if (!flagEmptyThirdColumn) {
                firstEmptyThirdColumn = row;
                flagEmptyThirdColumn = true;
            }
            if (fontMetric.width(currentVerseText + extendString) > minMesraWidth) {
                minMesraWidth = fontMetric.width(currentVerseText + extendString);
            }
            mesraItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            tableViewWidget->setItem(row, 1, mesraItem);
            if (MustHave2ndBandBeyt) {
                MustHave2ndBandBeyt = false;
            }
            break;

        case Left :
            spacerColumnIsPresent = true;
            if (!flagEmptyThirdColumn) {
                firstEmptyThirdColumn = row;
                flagEmptyThirdColumn = true;
            }
            if (fontMetric.width(currentVerseText + extendString) > minMesraWidth) {
                minMesraWidth = fontMetric.width(currentVerseText + extendString);
            }
            mesraItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            tableViewWidget->setItem(row, 3, mesraItem);
            break;

        case CenteredVerse1 :
            mesraItem->setTextAlignment(Qt::AlignCenter);
            tableViewWidget->setItem(row, 1, mesraItem);
            tableViewWidget->setSpan(row, 1, 1, 3);
            BandBeytNums++;
            MustHave2ndBandBeyt = true;
            break;

        case CenteredVerse2 :
            mesraItem->setTextAlignment(Qt::AlignCenter);
            tableViewWidget->setItem(row, 1, mesraItem);
            tableViewWidget->setSpan(row, 1, 1, 3);
            MustHave2ndBandBeyt = false;
            break;

        default:
            break;
        }

        if (spacerColumnIsPresent) {
            QTableWidgetItem* tmp = new QTableWidgetItem("");
            tmp->setFlags(Qt::NoItemFlags);
            tableViewWidget->setItem(row, 2, tmp);
        }
        break;//end of BeytPerLine

    case SteppedHemistichLine:
    case OneHemistichLine:
        switch (versePosition) {

        case Right :
            spacerColumnIsPresent = true;
            if (!flagEmptyThirdColumn) {
                firstEmptyThirdColumn = row;
                flagEmptyThirdColumn = true;
            }
            if (fontMetric.width(currentVerseText + extendString) > minMesraWidth) {
                minMesraWidth = fontMetric.width(currentVerseText + extendString);
            }

            if (CurrentViewStyle == SteppedHemistichLine) {
                mesraItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                tableViewWidget->setItem(row, 2, mesraItem);
            }
            else if (CurrentViewStyle == OneHemistichLine) {
                spacerColumnIsPresent = false;
                mesraItem->setTextAlignment(Qt::AlignCenter);
                tableViewWidget->setItem(row, 1, mesraItem);
                tableViewWidget->setSpan(row, 1, 1, 3);
            }

            if (MustHave2ndBandBeyt) {
                MustHave2ndBandBeyt = false;
            }
            break;

        case Left :
            spacerColumnIsPresent = true;
            if (!currentVerseText.isEmpty()) {
                ++row;
                *prow = row;
                tableViewWidget->insertRow(row);
            }
            if (!flagEmptyThirdColumn) {
                firstEmptyThirdColumn = row;
                flagEmptyThirdColumn = true;
            }
            if (fontMetric.width(currentVerseText + extendString) > minMesraWidth) {
                minMesraWidth = fontMetric.width(currentVerseText + extendString);
            }

            if (CurrentViewStyle == SteppedHemistichLine) {
                mesraItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                tableViewWidget->setItem(row, 2, mesraItem);
            }
            else if (CurrentViewStyle == OneHemistichLine) {
                spacerColumnIsPresent = false;
                mesraItem->setTextAlignment(Qt::AlignCenter);
                tableViewWidget->setItem(row, 1, mesraItem);
                tableViewWidget->setSpan(row, 1, 1, 3);
            }
            break;

        case CenteredVerse1 :
            mesraItem->setTextAlignment(Qt::AlignCenter);
            tableViewWidget->setItem(row, 1, mesraItem);
            tableViewWidget->setSpan(row, 1, 1, 3);
            BandBeytNums++;
            MustHave2ndBandBeyt = true;
            break;

        case CenteredVerse2 :
            mesraItem->setTextAlignment(Qt::AlignCenter);
            tableViewWidget->setItem(row, 1, mesraItem);
            tableViewWidget->setSpan(row, 1, 1, 3);
            MustHave2ndBandBeyt = false;
            break;

        default:
            break;
        }

        if (spacerColumnIsPresent) {
            QTableWidgetItem* tmp = new QTableWidgetItem("");
            tmp->setFlags(Qt::NoItemFlags);
            tableViewWidget->setItem(row, 1, tmp);

            tmp = new QTableWidgetItem("");
            tmp->setFlags(Qt::NoItemFlags);
            tableViewWidget->setItem(row, 3, tmp);
        }
        break;//end of AllMesrasCentered

    default:
        break;
    }// end of CurrentViewStyle switch
}

void SaagharWidget::createCustomContextMenu(const QPoint &pos)
{
    QTextEdit* textEdit = qobject_cast<QTextEdit*>(sender());
    if (!textEdit || !tableViewWidget || currentPoem == 0) {
        return;
    }

    emit createContextMenuRequested(textEdit->mapToParent(pos));
}
