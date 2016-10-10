/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2016 by S. Razi Alavizadeh                               *
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
#include "importermanager.h"
#include "txtimporter.h"
#include "importeroptionsdialog.h"
#include "databasebrowser.h"

#include <QDebug>
#include <QApplication>

ImporterManager* ImporterManager::s_importerManager = 0;

ImporterManager::ImporterManager()
    : m_importPathLabel(0)
{
    registerImporter("txt", new TxtImporter);
}

ImporterManager* ImporterManager::instance()
{
    if (!s_importerManager) {
        s_importerManager = new ImporterManager;
    }

    return s_importerManager;
}

ImporterManager::~ImporterManager()
{
    qDeleteAll(m_registeredImporters);

    delete s_importerManager;
    s_importerManager = 0;
}

ImporterInterface* ImporterManager::importer(const QString &id)
{
    ImporterInterface* importer = m_registeredImporters.value(id, 0);

    return importer;
}

bool ImporterManager::importerIsAvailable()
{
    return !m_registeredImporters.isEmpty();
}

bool ImporterManager::registerImporter(const QString &id, ImporterInterface* importer)
{
    if (!m_registeredImporters.contains(id)) {
        m_registeredImporters.insert(id, importer);

        return true;
    }

    return false;
}

void ImporterManager::unRegisterImporter(const QString &id)
{
    m_registeredImporters.take(id);
}

QStringList ImporterManager::availableFormats()
{
    QStringList formats;
    foreach (ImporterInterface* importer, m_registeredImporters) {
        formats << QString("%1 (*.%2)").arg(importer->readableName()).arg(importer->suffix());
    }

    return formats;
}
#include "outlinemodel.h"
#include "saagharwidget.h"
#include <QTreeView>
#include <QTreeWidget>
void ImporterManager::storeAsDataset(const CatContents &importData, bool storeAsGDB)
{
    m_importData.clear();
    m_importData = importData;

    if (m_importData.isNull()) {
        return;
    }


    if (storeAsGDB) {
        // ask for file name to save
    }

    m_importPath.clear();
    m_forceCreateNew = false;

    QDialog importPath(qApp->activeWindow());
    QVBoxLayout layout;
    QLabel infoLabel(tr("The poems will import to the following category:"), &importPath);
    m_importPathView = new QTreeWidget(&importPath);
    m_importPathView.data()->header()->hide();
    QTreeWidgetItem* rootItem = new QTreeWidgetItem(m_importPathView, QStringList(SaagharWidget::rootTitle()));
    m_importPathView.data()->addTopLevelItem(rootItem);
    m_importPathView.data()->setCurrentItem(rootItem);
    //m_importPathView.data()->setModel(OutlineModel::instance());
    m_importPathLabel = new QLabel(&importPath);
    QPushButton selectCat(tr("Select or Create a Poet or Book..."), &importPath);
    QPushButton importNow(tr("Import..."), &importPath);
    QPushButton clear(tr("Clear"), &importPath);
    importNow.hide();
    clear.hide();
    // selectCat.hide();
    layout.addWidget(&infoLabel);
    layout.addWidget(m_importPathView);
    layout.addWidget(m_importPathLabel);
    layout.addWidget(&selectCat);
    layout.addWidget(&clear);
    layout.addWidget(&importNow);
    importPath.setLayout(&layout);
    connect(&selectCat, SIGNAL(clicked(bool)), this, SLOT(importPathChanged()));
    connect(&clear, SIGNAL(clicked(bool)), this, SLOT(clearImportPath()));
    connect(&importNow, SIGNAL(clicked(bool)), this, SLOT(importHere()));
    connect(this, SIGNAL(importDialogDone()), &importPath, SLOT(accept()));
    // connect(m_importPathView.data()->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), &importNow, SLOT(show()));
    // connect(m_importPathView.data()->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), &selectCat, SLOT(show()));

    connect(&selectCat, SIGNAL(clicked(bool)), &importNow, SLOT(show()));
    connect(&selectCat, SIGNAL(clicked(bool)), &clear, SLOT(show()));
    importPath.exec();
//    foreach (const GanjoorPoem &poem, importData.poems) {
//        QList<GanjoorVerse> verses = importData.verses.value(poem._ID);
//        content += QString("Poem Title: %1\n----------------\n")
//                .arg(poem._Title);

//        foreach (const GanjoorVerse &verse, verses) {
//            content += QString("%1 - %2\n")
//                    .arg(verse._Order + 1).arg(verse._Text);
//        }
//        content += "\n=================================\n\n";
//    }
}

QString ImporterManager::convertTo(const CatContents &importData, ImporterManager::ConvertType type) const
{
    if (importData.isNull()) {
        return QObject::tr("<EMPTY PREVIEW>");
    }

    QString content;

    if (type == PlainText) {
        foreach (const GanjoorPoem &poem, importData.poems) {
            QList<GanjoorVerse> verses = importData.verses.value(poem._ID);

            if (verses.isEmpty()) {
                continue;
            }

            QStringList parentsTitles = importData.catParentsTitles(poem._CatID);

            content += QObject::tr("Top Level Categories: %1\n---\nPoem Title: %2\n----------------\n")
                       .arg(parentsTitles.isEmpty() ? tr("N/A") : parentsTitles.join(tr(" > ")))
                       .arg(poem._Title.isEmpty() ? tr("No poem title detected!") : poem._Title);

            foreach (const GanjoorVerse &verse, verses) {
                content += QString("%1 - %2\n")
                           .arg(verse._Order + 1).arg(verse._Text);
            }
            content += "\n=================================\n\n";
        }
    }
    else if (type == HtmlText) {
        QString toc = "<html><body><ul>";

        foreach (const GanjoorPoem &poem, importData.poems) {
            QList<GanjoorVerse> verses = importData.verses.value(poem._ID);

            if (verses.isEmpty()) {
                continue;
            }

            QStringList parentsTitles = importData.catParentsTitles(poem._CatID);

            toc += QString("<li><a href=\"#%1\"><i>%2</i></a>").arg(poem._ID).arg(poem._Title.isEmpty() ? tr("No poem title detected!") : poem._Title);
            content += QString("<br><h3>%1:</h3><center><br><h2><b><a id=\"%2\" name=\"%2\">%3</a></b></h2><br>--------------------------------<br></center>")
                       .arg(parentsTitles.isEmpty() ? tr("N/A") : parentsTitles.join(tr(" > ")))
                       .arg(poem._ID)
                       .arg(poem._Title.isEmpty() ? tr("No poem title detected!") : poem._Title);

            foreach (const GanjoorVerse &verse, verses) {
                QString openTags = "<p>";
                QString closeTags = "</p>";
                if (verse._Position == Paragraph) {
                    openTags = "<p style=\"margin-bottom: 0.2cm\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
                    closeTags = "</p>";
                }
                else if (verse._Position == Single) {
                    openTags = "<p style=\"font-size: 50%; color: #007;\">&nbsp;&nbsp;&nbsp;&nbsp;";
                    closeTags = "</p>";
                }
                else if (verse._Position == CenteredVerse1) {
                    openTags = "<center><p style=\"margin-top: 0.1cm\"><i>";
                    closeTags = "</i></p></center>";
                }
                else if (verse._Position == CenteredVerse2) {
                    openTags = "<center><p style=\"margin-bottom: 0.2cm\"><i>";
                    closeTags = "</i></p></center><br>";
                }
                else if (verse._Position == Right) {
                    openTags = "<center><p><i>";
                    closeTags = "</i></p></center>";
                }
                else if (verse._Position == Left) {
                    openTags = "<center><p style=\"margin-bottom: 1cm\"><i>";
                    closeTags = "</i></p></center><br>";
                }

                content += QString("%1%3%2")
                           .arg(openTags).arg(closeTags).arg(verse._Text);
            }

            content += "<hr>";
        }

        content = toc + "</ul><hr><hr>" + content + "</body></html>";
    }
    else if (type == EditingText) {
        content = convertToSED(importData);
    }

    return content;
}

static QString lineTypeFromPosition(VersePosition position)
{
    switch (position) {
    case Right:
        return "_CLASSIC_POEM_";
        break;
    case Left:
        return "_CLASSIC_POEM_";
        break;
    case CenteredVerse1:
        return "_CLASSIC_POEM_";
        break;
    case CenteredVerse2:
        return "_CLASSIC_POEM_";
        break;
    case Single:
        return "_SINGLE_";
        break;
    case Paragraph:
        return "_PARAGRAPH_";
        break;
    default:
        break;
    }
    return "_UNKNOWN_";
}

/*******************************************************************************/
// tags for Version-0.1 of Saaghar Easy Editable Dataset Format (SED)
// #SAAGHAR!SED!         // start of SED file
// #SED!LANGUAGE!        // SED file language
// #SED!CAT!START!       // start of cat
// #CAT!ID!              // ganjoor id of category
// #CAT!UID!             // unique id of category
// #CAT!CATID!           // ganjoor id of toplevel category of current category
// #CAT!CATUID!          // unique id of toplevel category of current category
// #CAT!TITLE!           // title of category
// #SED!POEM!START!      // start of poem
// #POEM!CATID!          // ganjoor id of toplevel category of current poem
// #POEM!CATUID!         // unique id of toplevel category of current poem
// #POEM!ID!             // ganjoor id of poem
// #POEM!UID!            // unique id of poem
// #POEM!TITLE!          // title of poem
// #POEM!VERSECOUNT!     // title of poem
// #SED!VERSES!START!    // start of verses
// #SED!VERSE!START!     // start of verse
// #VERSE!ORDER!         // verse order within poem
// #VERSE!POSITION!      // verse type and position
// #VERSE!TEXT!          // verse text
// #SED!VERSE!END!       // end of verse
// #SED!POEM!END!        // end of poem
// #SED!CAT!END!         // end of cat
/*******************************************************************************/
QString ImporterManager::convertToSED(const CatContents &importData) const
{
    QString content;

    QString poetUID = "poetUID";
    QString catUID = "catUID";
    QString poetTitle = "poetTitle";
    QString catTitle = "catTitle";

    content += "#SAAGHAR!SED!v0.1\n";
//    content += QString("#SED!LANGUAGE!FA_IR\n#SED!CAT!START!#CAT!UID!%1#CAT!CATID!0\n#CAT!TITLE!%2\n"
//                       "#SED!CAT!START!#CAT!UID!%3#CAT!CATUID!%1\n#CAT!TITLE!%4\n")
//            .arg(poetUID).arg(poetTitle).arg(catUID).arg(catTitle);

    content += "######################\n######################\n";

    foreach (const GanjoorPoem &poem, importData.poems) {
        QList<GanjoorVerse> verses = importData.verses.value(poem._ID);
//        content += QString("#SED!POEM!START!#POEM!CATUID!%1#POEM!ID!%2#POEM!VERSECOUNT!%4\n#POEM!TITLE!%3\n")
//                .arg(catUID).arg(poem._ID).arg(poem._Title).arg(verses.count());

        QStringList parentsTitles = importData.catParentsTitles(poem._CatID);

        content += QString("#CAT!TITLE!%1\n###\n#POEM!TITLE!%2\n")
                   .arg(parentsTitles.isEmpty() ? "NO_CAT" : parentsTitles.join("\n#CAT!TITLE!"))
                   .arg(poem._Title.isEmpty() ? "UNKNOWN_POEM_TITLE" : poem._Title);

        QString lastPosition;
        // content += QString("#SED!VERSES!START!\n");
        foreach (const GanjoorVerse &verse, verses) {
            if (lastPosition != lineTypeFromPosition(verse._Position)) {
                lastPosition = lineTypeFromPosition(verse._Position);
                content += QString("###\n#VERSE!POSITION!%1\n").arg(lastPosition);
            }
            content += QString("%1\n").arg(verse._Text);
//            content += QString("#SED!VERSE!START!#VERSE!ORDER!%1#VERSE!POSITION!%2\n#VERSE!TEXT!%3\n")
//                    .arg(verse._Order).arg(versePositionToString(verse._Position)).arg(verse._Text);
        }
        content += "######################\n";
    }

    return content;
}

bool ImporterManager::initializeImport()
{
    ImporterOptionsDialog* optionDialog = new ImporterOptionsDialog(qApp->activeWindow());

    return optionDialog->exec() == QDialog::Accepted;
}
#include "selectcreatedialog.h"
#include <QInputDialog>
void ImporterManager::importPathChanged()
{
    QPushButton* selectCat = qobject_cast<QPushButton*>(sender());
    if (m_importPathView && !m_importPathView.data()->selectionModel()->selectedIndexes().isEmpty()) {
        SelectCreateDialog selectCatDialog(qApp->activeWindow());
        selectCatDialog.setForceCreate(m_forceCreateNew);
        selectCatDialog.adjustSize();

        if (selectCatDialog.exec() == QDialog::Accepted) {
            if (selectCatDialog.createNewCat()) {
                m_forceCreateNew = true;
                QTreeWidgetItem* childItem = new QTreeWidgetItem(m_importPathView.data()->selectedItems().at(0)
                        ? m_importPathView.data()->selectedItems().at(0)
                        : m_importPathView.data()->topLevelItem(0), QStringList(selectCatDialog.newTitle()));
                GanjoorCat cat;
                cat._ID = -1;
                cat._Text = childItem->text(0);
                childItem->setData(0, OutlineModel::CategoryRole, QVariant::fromValue(cat));
                m_importPathView.data()->selectedItems().at(0)->addChild(childItem);
                m_importPathView.data()->setCurrentItem(childItem);
            }
            else {
                m_importPath = selectCatDialog.selectedPath();
                QList<QTreeWidgetItem*> children = m_importPathView.data()->topLevelItem(0)->takeChildren();
                qDeleteAll(children);
                QTreeWidgetItem* rootItem = m_importPathView.data()->topLevelItem(0);
                foreach (const GanjoorCat &cat, selectCatDialog.selectedCatPath(true)) {
                    QTreeWidgetItem* childItem = new QTreeWidgetItem(rootItem, QStringList(cat._Text));
                    childItem->setData(0, OutlineModel::CategoryRole, QVariant::fromValue(cat));
                    rootItem->addChild(childItem);
                    m_importPathView.data()->setCurrentItem(childItem);
                    rootItem = childItem;
                }
            }
        }
//        QString title = QInputDialog::getText(qApp->activeWindow(), tr("Create New Category"), tr("Enter title:"));
//        m_importPathView.data()->model()->insertRow(0, m_importPathView.data()->selectionModel()->selectedIndexes().at(0));
    }

    return;
    if (m_importPathLabel) {
        SelectCreateDialog selectCatDialog(qApp->activeWindow(), m_importPath);
        selectCatDialog.setForceCreate(m_forceCreateNew);

        if (selectCatDialog.exec() == QDialog::Accepted) {
            if (selectCatDialog.createNewCat()) {
                m_forceCreateNew = true;
            }
            else {
                m_importPath = selectCatDialog.selectedPath();
            }
        }
//        QDialog selectCatDialog(qApp->activeWindow());
//        QVBoxLayout layout;
//        QLabel infoLabel(tr("The poems will import to the following category:"), &importPath);

//        m_importPathLabel.data()->setText(m_importPathLabel.data()->text() + "\nADD PATH");
//        selectCat->setText(tr("Select or Create a Category or Book..."));
    }
}

void ImporterManager::clearImportPath()
{
    if (m_importPathView) {
        QPushButton* clear = qobject_cast<QPushButton*>(sender());
        QList<QTreeWidgetItem*> children = m_importPathView.data()->topLevelItem(0)->takeChildren();
        qDeleteAll(children);
        m_importPathView.data()->setCurrentItem(m_importPathView.data()->topLevelItem(0));
        m_forceCreateNew = false;
        clear->hide();
    }
}
#include <QMessageBox>
void ImporterManager::importHere()
{
    if (m_importData.isNull() || !m_importPathView || m_importPathView.data()->selectedItems().isEmpty()) {
        return;
    }

    QList<GanjoorCat> catPath;
    QStringList titlePath;
    QTreeWidgetItem* homeItem = m_importPathView.data()->topLevelItem(0);
    QTreeWidgetItem* item = m_importPathView.data()->selectedItems().at(0);
    while (item && item != homeItem) {
        GanjoorCat cat = item->data(0, OutlineModel::CategoryRole).value<GanjoorCat>();
        qDebug() << "\n@@@@@@@@@@@@@@@@@@@@@@@@\n" << __LINE__ << __FUNCTION__ << "\n"
                 << cat._Text << "\n"
                 << cat._ID << "\n"
                 << cat._ParentID << "\n"
                 << cat._PoetID << "\n"
                 << "\n@@@@@@@@@@@@@@@@@@@@@@@@\n" ;
        catPath.prepend(cat);
        titlePath.prepend(item->text(0));
        item = item->parent();
    }
    qDebug() << "\n===========\n" << __LINE__ << __FUNCTION__ << m_importPathView.data()->selectionModel()->selectedIndexes();
    if (QMessageBox::Cancel == QMessageBox::information(
                qApp->activeWindow(), tr("Import"),
                tr("Data will import as subsections of the following category:\n%1")
                .arg(titlePath.join(tr(" > \n\t"))), // FIXME: Add LRM
                QMessageBox::Ok, QMessageBox::Cancel)) {
        return;
    }

    DatabaseBrowser::instance()->storeAsDataset(m_importData, catPath);
    m_importData.clear();

    emit importDialogDone();
}
