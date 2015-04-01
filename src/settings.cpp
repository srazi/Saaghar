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

#include "settings.h"
#include "saagharwindow.h"
#include "saagharapplication.h"
#include "settingsmanager.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QImageReader>

QString Settings::s_currentIconPath;

Settings::Settings(SaagharWindow* parent)
    : QDialog(parent)
    , ui(new Ui::Settings)
{
    ui->setupUi(this);

    connect(ui->globalFontColorGroupBox, SIGNAL(toggled(bool)), this, SLOT(setUnCheckedOtherFontColorGroupBox(bool)));
    connect(ui->advancedFontColorGroupBox, SIGNAL(toggled(bool)), this, SLOT(setUnCheckedOtherFontColorGroupBox(bool)));

    backgroundFontColor = new FontColorSelector(VAR("SaagharWidget/Fonts/OutLine").value<QFont>(),
            SaagharWidget::backgroundColor, ui->groupBox_2);
    backgroundFontColor->setObjectName(QString::fromUtf8("backgroundFontColor"));
    backgroundFontColor->setColorSelector(true);
    ui->horizontalLayout_6->addWidget(backgroundFontColor);

    matchedFontColor = new FontColorSelector(VAR("SaagharWidget/Fonts/Default").value<QFont>(),
            SaagharWidget::matchedTextColor, ui->groupBox_2);
    matchedFontColor->setObjectName(QString::fromUtf8("matchedFontColor"));
    matchedFontColor->setColorSelector(true);
    ui->horizontalLayout_5->addWidget(matchedFontColor);


    outlineFontColor = new FontColorSelector(VAR("SaagharWidget/Fonts/OutLine").value<QFont>(),
            VAR("SaagharWidget/Colors/OutLine").value<QColor>(),
            ui->groupBox_2);
    outlineFontColor->setObjectName(QString::fromUtf8("outlineFontColor"));
    ui->outlineLayout->addWidget(outlineFontColor);

    globalTextFontColor = new FontColorSelector(VAR("SaagharWidget/Fonts/Default").value<QFont>(),
            VAR("SaagharWidget/Colors/Default").value<QColor>(),
            ui->globalFontColorGroupBox);
    globalTextFontColor->setObjectName(QString::fromUtf8("globalTextFontColor"));
    ui->gridLayout_16->addWidget(globalTextFontColor, 0, 1, 1, 1);


    sectionNameFontColor = new FontColorSelector(VAR("SaagharWidget/Fonts/SectionName").value<QFont>(),
            VAR("SaagharWidget/Colors/SectionName").value<QColor>(),
            ui->advancedFontColorGroupBox);
    sectionNameFontColor->setObjectName(QString::fromUtf8("sectionNameFontColor"));
    ui->gridLayout_9->addWidget(sectionNameFontColor, 0, 1, 1, 1);

    titlesFontColor = new FontColorSelector(VAR("SaagharWidget/Fonts/Titles").value<QFont>(),
                                            VAR("SaagharWidget/Colors/Titles").value<QColor>(),
                                            ui->advancedFontColorGroupBox);
    titlesFontColor->setObjectName(QString::fromUtf8("titlesFontColor"));
    ui->gridLayout_9->addWidget(titlesFontColor, 1, 1, 1, 1);

    numbersFontColor = new FontColorSelector(VAR("SaagharWidget/Fonts/Numbers").value<QFont>(),
            VAR("SaagharWidget/Colors/Numbers").value<QColor>(),
            ui->advancedFontColorGroupBox);
    numbersFontColor->setObjectName(QString::fromUtf8("numbersFontColor"));
    ui->gridLayout_9->addWidget(numbersFontColor, 2, 1, 1, 1);

    poemTextFontColor = new FontColorSelector(VAR("SaagharWidget/Fonts/PoemText").value<QFont>(),
            VAR("SaagharWidget/Colors/PoemText").value<QColor>(),
            ui->advancedFontColorGroupBox);
    poemTextFontColor->setObjectName(QString::fromUtf8("poemTextFontColor"));
    ui->gridLayout_9->addWidget(poemTextFontColor, 3, 1, 1, 1);

    proseTextFontColor = new FontColorSelector(VAR("SaagharWidget/Fonts/ProseText").value<QFont>(),
            VAR("SaagharWidget/Colors/ProseText").value<QColor>(),
            ui->advancedFontColorGroupBox);
    proseTextFontColor->setObjectName(QString::fromUtf8("proseTextFontColor"));
    ui->gridLayout_9->addWidget(proseTextFontColor, 4, 1, 1, 1);

    connect(ui->pushButtonApply, SIGNAL(clicked()), parent, SLOT(applySettings()));

    ui->checkBoxIconTheme->setChecked(VARB("Icon Theme State"));
    ui->lineEditIconTheme->setEnabled(VARB("Icon Theme State"));
    ui->pushButtonIconTheme->setEnabled(VARB("Icon Theme State"));
    ui->lineEditIconTheme->setText(VARS("Icon Theme Path"));
    connect(ui->pushButtonIconTheme, SIGNAL(clicked()), this, SLOT(browseForIconTheme()));

    // force emitting toggle signal
    ui->globalFontColorGroupBox->setChecked(!VARB("Global Font"));
    ui->globalFontColorGroupBox->setChecked(VARB("Global Font"));

    connect(ui->checkBoxBackground, SIGNAL(toggled(bool)), backgroundFontColor, SLOT(disableAll(bool)));
    backgroundFontColor->disableAll(ui->checkBoxBackground->isChecked());
    ui->labelBackgroundColor->setDisabled(ui->checkBoxBackground->isChecked());

    setupTaskManagerUi();
}

Settings::~Settings()
{
    delete ui;
}

void Settings::addActionToToolbarTable()
{
    QList<QTableWidgetItem*> allActionItemList = ui->tableWidgetAllActions->selectedItems();
    if (allActionItemList.isEmpty()) {
        return;
    }
    QList<QTableWidgetItem*> toolbarItemList = ui->tableWidgetToolBarActions->selectedItems();
    int currentRowInToolbarAction = ui->tableWidgetToolBarActions->rowCount();
    if (!toolbarItemList.isEmpty()) {
        currentRowInToolbarAction = toolbarItemList.at(0)->row() + 1;
    }

    int currentRowInAllAction = allActionItemList.at(0)->row();
    QTableWidgetItem* item = new QTableWidgetItem(*allActionItemList.at(0));
    if (!item) {
        return;
    }

    ui->tableWidgetToolBarActions->insertRow(currentRowInToolbarAction);
    ui->tableWidgetToolBarActions->setItem(currentRowInToolbarAction, 0, item);
    if (!item->data(Qt::UserRole + 1).toString().contains("separator", Qt::CaseInsensitive)) {
        ui->tableWidgetAllActions->removeRow(currentRowInAllAction);
    }
}

void Settings::removeActionFromToolbarTable()
{
    QList<QTableWidgetItem*> toolbarItemList = ui->tableWidgetToolBarActions->selectedItems();
    if (toolbarItemList.isEmpty()) {
        return;
    }

    int currentRowInToolbarAction = toolbarItemList.at(0)->row();

    QTableWidgetItem* item = new QTableWidgetItem(*toolbarItemList.at(0));
    if (!item) {
        return;
    }

    if (!item->data(Qt::UserRole + 1).toString().contains("separator", Qt::CaseInsensitive)) {
        ui->tableWidgetAllActions->insertRow(ui->tableWidgetAllActions->rowCount());
        ui->tableWidgetAllActions->setItem(ui->tableWidgetAllActions->rowCount() - 1, 0, item);
        ui->tableWidgetAllActions->sortByColumn(0, Qt::AscendingOrder);
    }

    ui->tableWidgetToolBarActions->removeRow(currentRowInToolbarAction);
}

void Settings::bottomAction()
{
    replaceWithNeighbor(1);
}

void Settings::topAction()
{
    replaceWithNeighbor(-1);
}

void Settings::replaceWithNeighbor(int neighbor)
{
    QList<QTableWidgetItem*> itemList = ui->tableWidgetToolBarActions->selectedItems();
    if (itemList.isEmpty()) {
        return;
    }
    int row = itemList.at(0)->row();
    QTableWidgetItem* neighborItem = ui->tableWidgetToolBarActions->item(row + neighbor, 0);
    if (neighborItem) {
        QTableWidgetItem* firstItem = new  QTableWidgetItem(*itemList.at(0));
        QTableWidgetItem* secondItem = new  QTableWidgetItem(*neighborItem);
        ui->tableWidgetToolBarActions->setItem(row + neighbor, 0, firstItem);
        ui->tableWidgetToolBarActions->setItem(row, 0, secondItem);
        ui->tableWidgetToolBarActions->selectRow(row + neighbor);
        ui->tableWidgetToolBarActions->update();
    }
}

void Settings::setupTaskManagerUi()
{
    ui->comboBoxMode->clear();
    ui->comboBoxMode->insertItem(0, tr("Slow"), "SLOW");
    ui->comboBoxMode->insertItem(1, tr("Normal (Recommended)"), "NORMAL");
    ui->comboBoxMode->insertItem(2, tr("Fast"), "FAST");

    ui->comboBoxNotification->clear();
    ui->comboBoxNotification->insertItem(0, tr("Disabled"), ProgressManager::Disabled);
    ui->comboBoxNotification->insertItem(1, tr("Saaghar Bottom Right"), ProgressManager::AppBottomRight);
    ui->comboBoxNotification->insertItem(2, tr("Saaghar Bottom Left"), ProgressManager::AppBottomLeft);
    ui->comboBoxNotification->insertItem(3, tr("Desktop Bottom Right"), ProgressManager::DesktopBottomRight);
    ui->comboBoxNotification->insertItem(4, tr("Desktop Top Right"), ProgressManager::DesktopTopRight);

    ui->groupBoxTaskManager->setChecked(VARB("TaskManager"));
    ui->comboBoxMode->setCurrentIndex(ui->comboBoxMode->findData(VAR("TaskManager/Mode")));
    ui->comboBoxNotification->setCurrentIndex(ui->comboBoxNotification->findData(VAR("TaskManager/Notification")));
}

void Settings::initializeActionTables(const QMap<QString, QAction*> &actionsMap, const QStringList &toolBarItems)
{
    //table for all actions
    ui->tableWidgetAllActions->setRowCount(1);

    QTableWidgetItem* item = new QTableWidgetItem("------" + tr("Separator") + "------");
    item->setData(Qt::UserRole + 1, "separator");
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetAllActions->setItem(0, 0, item);
    if (!actionsMap.isEmpty()) {
        QMap<QString, QAction*>::const_iterator it = actionsMap.constBegin();
        int index = 1;
        while (it != actionsMap.constEnd()) {
            if (toolBarItems.contains(it.key(), Qt::CaseInsensitive) || !it.value()) {
                ++it;
                continue;
            }
            QString text = it.value()->text();
            text.remove("&");
            item = new QTableWidgetItem(text);
            item->setData(Qt::UserRole + 1, it.key());
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            item->setIcon(it.value()->icon().isNull() ? QIcon(":/resources/images/empty.png") : it.value()->icon());
            ui->tableWidgetAllActions->insertRow(index);
            ui->tableWidgetAllActions->setItem(index, 0, item);
            ++it;
            ++index;
        }
        ui->tableWidgetAllActions->resizeRowsToContents();
    }

    //table for main toolbar actions
    ui->tableWidgetToolBarActions->setRowCount(toolBarItems.size());
    for (int i = 0; i < toolBarItems.size(); ++i) {
        QString  actionString = toolBarItems.at(i);
        if (actionString.contains("separator", Qt::CaseInsensitive)) {
            item = new QTableWidgetItem("------" + tr("Separator") + "------");
            item->setData(Qt::UserRole + 1, "separator");
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            item->setTextAlignment(Qt::AlignCenter);
        }
        else {
            QAction* action = actionsMap.value(actionString);
            if (action) {
                QString text = action->text();
                text.remove("&");
                item = new QTableWidgetItem(text);
                item->setData(Qt::UserRole + 1, actionString);
                item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                item->setIcon(action->icon().isNull() ? QIcon(":/resources/images/empty.png") : action->icon());
            }
            else {
                continue;
            }
        }
        ui->tableWidgetToolBarActions->setItem(i, 0, item);
    }
    ui->tableWidgetToolBarActions->resizeRowsToContents();

    connect(ui->tableWidgetToolBarActions, SIGNAL(itemSelectionChanged()), this, SLOT(tableToolBarActionsEntered()));
    connect(ui->tableWidgetToolBarActions, SIGNAL(cellPressed(int,int)), this, SLOT(tableToolBarActionsEntered()));
    connect(ui->tableWidgetAllActions, SIGNAL(cellPressed(int,int)), this, SLOT(tableAllActionsEntered()));
}

void Settings::tableToolBarActionsEntered()
{
    QList<QTableWidgetItem*> itemList = ui->tableWidgetToolBarActions->selectedItems();
    int row = 0;
    if (!itemList.isEmpty()) {
        row = itemList.at(0)->row();
    }
    ui->pushButtonActionAdd->setEnabled(false);
    ui->pushButtonActionRemove->setEnabled(true);
    if (row == 0) {
        ui->pushButtonActionTop->setEnabled(false);
    }
    else {
        ui->pushButtonActionTop->setEnabled(true);
    }

    if (row == ui->tableWidgetToolBarActions->rowCount() - 1) {
        ui->pushButtonActionBottom->setEnabled(false);
    }
    else {
        ui->pushButtonActionBottom->setEnabled(true);
    }
}

void Settings::tableAllActionsEntered()
{
    ui->pushButtonActionAdd->setEnabled(true);
    ui->pushButtonActionRemove->setEnabled(false);
    ui->pushButtonActionTop->setEnabled(false);
    ui->pushButtonActionBottom->setEnabled(false);
}

void Settings::browseForDataBasePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Add Path For Data Base"), ui->lineEditDataBasePath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        dir.replace(QString("\\"), QString("/"));
        if (!dir.endsWith('/')) {
            dir += "/";
        }
        QString currentPath = ui->lineEditDataBasePath->text();
        if (!currentPath.isEmpty()) {
            currentPath += ";";
        }
//      ui->lineEditDataBasePath->setText(currentPath+dir);
        ui->lineEditDataBasePath->setText(dir);//in this version Saaghar just use its first search path
    }
}

void Settings::browseForBackground()
{
    QList<QByteArray> formatList = QImageReader::supportedImageFormats();
    QString imageFormats = "";
    for (int i = 0; i < formatList.size(); ++i) {
        QString tmp = QString(formatList.at(i));
        if (!tmp.isEmpty()) {
            imageFormats += "*." + QString(formatList.at(i));
        }
        if (i < formatList.size() - 1) {
            imageFormats += " ";
        }
    }

    QString location = QFileDialog::getOpenFileName(this, tr("Browse Background Image"), ui->lineEditBackground->text(), "Image files (" + imageFormats + ")"); //*.png *.bmp)");
    if (!location.isEmpty()) {
        location.replace(QString("\\"), QString("/"));
        ui->lineEditBackground->setText(location);
    }
}

void Settings::browseForIconTheme()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Browse Theme Directory"), ui->lineEditIconTheme->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        dir.replace(QString("\\"), QString("/"));
        if (!dir.endsWith('/')) {
            dir += "/";
        }
        ui->lineEditIconTheme->setText(dir);
    }
}

void Settings::setUnCheckedOtherFontColorGroupBox(bool unChecked)
{
    if (sender() == ui->advancedFontColorGroupBox) {
        ui->globalFontColorGroupBox->setChecked(!unChecked);
    }
    else if (sender() == ui->globalFontColorGroupBox) {
        ui->advancedFontColorGroupBox->setChecked(!unChecked);
    }
}

QString Settings::currentIconThemePath()
{
    if (s_currentIconPath.isEmpty()) {
        if (VARB("Icon Theme State") && !VARS("Icon Theme Path").isEmpty()) {
            s_currentIconPath = VARS("Icon Theme Path");
        }
        else {
            s_currentIconPath = QLatin1String(":/resources/iconsets/default/");
        }
    }

    return s_currentIconPath;
}

void Settings::accept()
{
    applySettings();

    QDialog::accept();
}

void Settings::applySettings()
{
    VAR_DECL("Icon Theme State", ui->checkBoxIconTheme->isChecked());
    VAR_DECL("Icon Theme Path", ui->lineEditIconTheme->text());
    s_currentIconPath.clear();

    const bool storeTaskManagerSettings = ui->groupBoxTaskManager->isChecked();
    VAR_DECL("TaskManager", storeTaskManagerSettings);

    if (storeTaskManagerSettings) {
        VAR_DECL("TaskManager/Mode", ui->comboBoxMode->itemData(ui->comboBoxMode->currentIndex()));
        VAR_DECL("TaskManager/Notification", ui->comboBoxNotification->itemData(ui->comboBoxNotification->currentIndex()));
    }
}


/*******************************
// class CustomizeRandomDialog
********************************/
CustomizeRandomDialog::CustomizeRandomDialog(QWidget* parent, bool openInNewTab) : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    setupui();
    this->setFixedSize(this->sizeHint());
    connect(pushButtonOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    checkBox->setChecked(openInNewTab);
}

CustomizeRandomDialog::~CustomizeRandomDialog()
{
}

void CustomizeRandomDialog::setupui()
{
    if (this->objectName().isEmpty()) {
        this->setObjectName(QString::fromUtf8("this"));
    }
    this->resize(336, 116);
    gridLayout = new QGridLayout(this);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    verticalLayout = new QVBoxLayout();
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    checkBox = new QCheckBox(this);
    checkBox->setObjectName(QString::fromUtf8("checkBox"));

    verticalLayout->addWidget(checkBox);

    label = new QLabel(this);
    label->setObjectName(QString::fromUtf8("label"));

    verticalLayout->addWidget(label);

    selectRandomRange = new QMultiSelectWidget(this);
    selectRandomRange->setObjectName(QString::fromUtf8("selectRandomRange"));
    selectRandomRange->getComboWidgetInstance()->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    verticalLayout->addWidget(selectRandomRange->getComboWidgetInstance());

    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(horizontalSpacer);

    pushButtonOk = new QPushButton(this);
    pushButtonOk->setObjectName(QString::fromUtf8("pushButtonOk"));

    horizontalLayout->addWidget(pushButtonOk);

    pushButtonCancel = new QPushButton(this);
    pushButtonCancel->setObjectName(QString::fromUtf8("pushButtonCancel"));

    horizontalLayout->addWidget(pushButtonCancel);


    verticalLayout->addLayout(horizontalLayout);


    gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);


    retranslateUi();
}

void CustomizeRandomDialog::retranslateUi()
{
    this->setWindowTitle(tr("Customize Faal & Random"));
    checkBox->setText(tr("Open in new Tab"));
    label->setText(tr("Select Random Range:"));
    pushButtonOk->setText(tr("Ok"));
    pushButtonCancel->setText(tr("Cancel"));
}


void CustomizeRandomDialog::acceptSettings(bool* checked)
{
    *checked = checkBox->isChecked();
}

/*******************************
// class FontColorSelector
********************************/

#include <QFontDialog>

FontColorSelector::FontColorSelector(const QFont &defaultFont, const QColor &defaultColor, QWidget* parent, const QString &sampleText)
    : QWidget(parent)
{
    sampleLabel = new QLabel(sampleText.isEmpty() ? tr("Font Sample!") : sampleText, parent);

    sampleLabel->setMaximumHeight(30);
    sampleLabel->setMaximumWidth(400);

    fontInfo = new QLabel(parent);

    fontSelector = new QPushButton(tr("Set Font..."), parent);
    colorSelector = new QPushButton(parent);
    colorSelector->setFlat(true);
    //colorSelector->setStyleSheet("QPushButton{border: 1px solid gray; border-radius: 3px;}");
    colorSelector->setFixedSize(24, 24);
    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect(fontSelector, SIGNAL(clicked()), this, SLOT(selectFont()));
    connect(colorSelector, SIGNAL(clicked()), this, SLOT(selectColor()));

    //bold = new QCheckBox("Bold", this);
    //italic = new QCheckBox("Italic", this);

    setSampleFont(defaultFont);

    if (defaultColor.isValid()) {
        setColor(defaultColor);
    }
    else {
        setColor(color());
    }

    hLayout = new QHBoxLayout(0);
    hLayout->setSpacing(1);
    hLayout->setContentsMargins(1, 1, 1, 1);
    hLayout->addWidget(sampleLabel);
    hLayout->addItem(horizontalSpacer);
    hLayout->addWidget(fontInfo);
    hLayout->addWidget(colorSelector);
    hLayout->addWidget(fontSelector);
    //hLayout->addWidget(bold);
    //hLayout->addWidget(italic);

    this->setLayout(hLayout);

    if (parent) {
        connect(parent, SIGNAL(toggled(bool)), this, SLOT(enableAll(bool)));
    }
}

QColor FontColorSelector::color()
{
    return m_color;
}

void FontColorSelector::setColorSelector(bool colorSelector)
{
    sampleLabel->setHidden(colorSelector);
    fontInfo->setHidden(colorSelector);
    fontSelector->setHidden(colorSelector);
}

void FontColorSelector::setColor(const QColor &color)
{
    if (!color.isValid()) {
        return;
    }

    m_color = color;

    colorSelector->setStyleSheet(QString("QPushButton{background: %1; border: 1px solid gray; border-radius: 3px;}")
                                 .arg(color.name()));
    sampleLabel->setStyleSheet(QString("QLabel{color: %1; border: 1px solid gray; border-radius: 3px;}")
                               .arg(color.name()));
}

void FontColorSelector::setSampleFont(const QFont &font)
{
    sampleLabel->setFont(font);

    fontInfo->setText(" " + font.family() + ", " + QString::number(font.pointSize()) + " ");
    QFont fontInfoLabelFont(fontInfo->font());
    fontInfoLabelFont.setBold(font.bold());
    fontInfoLabelFont.setItalic(font.italic());
    fontInfo->setFont(fontInfoLabelFont);
}

void FontColorSelector::selectFont()
{
    disconnect(fontSelector, SIGNAL(clicked()), this, SLOT(selectFont()));
    //bool ok;
    QFont tmp = sampleFont();
    //QFont font = QFontDialog::getFont(&ok, sampleFont(), this);
    QFontDialog fontDialog(tmp, this);
    connect(&fontDialog, SIGNAL(currentFontChanged(QFont)), this, SLOT(setSampleFont(QFont)));
    //connect(&fontDialog, SIGNAL(currentFontChanged(QFont)), this, SLOT(setSampleFont(QFont)));

    if (fontDialog.exec()) {
        setSampleFont(fontDialog.selectedFont());
    }
    else {
        //maybe after emmitting signals font has been changed
        setSampleFont(tmp);
    }
    connect(fontSelector, SIGNAL(clicked()), this, SLOT(selectFont()));
}

void FontColorSelector::selectColor()
{
    disconnect(colorSelector, SIGNAL(clicked()), this, SLOT(selectColor()));
    QColor color = QColorDialog::getColor(m_color, this);
    setColor(color);
    connect(colorSelector, SIGNAL(clicked()), this, SLOT(selectColor()));
}

void FontColorSelector::enableAll(bool enabled)
{
    if (enabled) {
        connect(fontSelector, SIGNAL(clicked()), this, SLOT(selectFont()));
        connect(colorSelector, SIGNAL(clicked()), this, SLOT(selectColor()));
        setColor(m_color);
    }
    else {
        disconnect(fontSelector, SIGNAL(clicked()), this, SLOT(selectFont()));
        disconnect(colorSelector, SIGNAL(clicked()), this, SLOT(selectColor()));
        colorSelector->setStyleSheet(QString("QPushButton{background: lightgray; border: 1px solid gray; border-radius: 3px;}"));
        sampleLabel->setStyleSheet(QString("QLabel{color: lightgray; border: 1px solid gray; border-radius: 3px;}"));
    }
}

void FontColorSelector::disableAll(bool disable)
{
    enableAll(!disable);
}
