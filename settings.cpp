/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2012 by S. Razi Alavizadeh                          *
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

#include <QColorDialog>
#include <QFileDialog>
#include <QImageReader>

QHash<QString, QVariant> Settings::VariablesHash = QHash<QString, QVariant>();
QHash<QString, QVariant> Settings::hashFonts = QHash<QString, QVariant>();
QHash<QString, QVariant> Settings::hashColors = QHash<QString, QVariant>();

Settings::Settings(QWidget *parent) :	QDialog(parent), ui(new Ui::Settings)
{
	ui->setupUi(this);

	connect(ui->globalFontColorGroupBox, SIGNAL(toggled(bool)), this, SLOT(setUnCheckedOtherFontColorGroupBox(bool)));
	connect(ui->advancedFontColorGroupBox, SIGNAL(toggled(bool)), this, SLOT(setUnCheckedOtherFontColorGroupBox(bool)));


	ui->globalFontColorGroupBox->setChecked(Settings::READ("Global Font", false).toBool());
	//QHash<QString, QVariant> defaultFonts;
	//QFont fnt;
//	defaultFonts.insert("default", fnt);
//	defaultFonts.insert("PoemText", fnt);
//	defaultFonts.insert("ProseText", fnt);
//	defaultFonts.insert("SectionName", fnt);
//	defaultFonts.insert("PoemTitle", QFont(fnt.family(),fnt.pointSize()+2));
//	defaultFonts.insert("BeytNumber", fnt);
	//QHash<QString, QVariant> defaultColors;
//	defaultColors.insert("default", QColor(0x23,0x65, 0xFF));
//	defaultColors.insert("PoemText", QColor(0x23,0x65, 0xFF));
//	defaultColors.insert("ProseText", QColor(0x23,0x65, 0xFF));
//	defaultColors.insert("SectionName", QColor(0x23,0x65, 0xFF));
//	defaultColors.insert("PoemTitle", QColor(0x23,0x65, 0xFF));
//	defaultColors.insert("BeytNumber", QColor(0x23,0x65, 0xFF));
	
//	QHash<QString, QVariant> tmphashFonts = Settings::READ("Fonts Hash", QVariant(defaultFonts)).toHash();
//	QHash<QString, QVariant> tmphashColors = Settings::READ("Colors Hash", QVariant(defaultColors)).toHash();

	outlineFontColor = new FontColorSelector(getFromFonts(OutLineFontColor, false),
												getFromColors(OutLineFontColor, false),
													ui->groupBox_2);
	outlineFontColor->setObjectName(QString::fromUtf8("outlineFontColor"));
	ui->outlineLayout->addWidget(outlineFontColor);

	globalTextFontColor = new FontColorSelector(getFromFonts(DefaultFontColor, false),
												getFromColors(DefaultFontColor, false),
													ui->globalFontColorGroupBox);
	globalTextFontColor->setObjectName(QString::fromUtf8("globalTextFontColor"));
	ui->gridLayout_16->addWidget(globalTextFontColor, 0, 1, 1, 1);


	sectionNameFontColor = new FontColorSelector(getFromFonts(SectionNameFontColor, false),
												 getFromColors(SectionNameFontColor, false),
													 ui->advancedFontColorGroupBox);
	 sectionNameFontColor->setObjectName(QString::fromUtf8("sectionNameFontColor"));
	 ui->gridLayout_9->addWidget(sectionNameFontColor, 0, 1, 1, 1);

	titlesFontColor = new FontColorSelector(getFromFonts(TitlesFontColor, false),
											getFromColors(TitlesFontColor, false),
												ui->advancedFontColorGroupBox);
	titlesFontColor->setObjectName(QString::fromUtf8("titlesFontColor"));
	ui->gridLayout_9->addWidget(titlesFontColor, 1, 1, 1, 1);

	numbersFontColor = new FontColorSelector(getFromFonts(NumbersFontColor, false),
											 getFromColors(NumbersFontColor, false),
												 ui->advancedFontColorGroupBox);
	numbersFontColor->setObjectName(QString::fromUtf8("numbersFontColor"));
	ui->gridLayout_9->addWidget(numbersFontColor, 2, 1, 1, 1);

	poemTextFontColor = new FontColorSelector(getFromFonts(PoemTextFontColor, false),
												getFromColors(PoemTextFontColor, false),
													ui->advancedFontColorGroupBox);
	poemTextFontColor->setObjectName(QString::fromUtf8("poemTextFontColor"));
	ui->gridLayout_9->addWidget(poemTextFontColor, 3, 1, 1, 1);

	proseTextFontColor = new FontColorSelector(getFromFonts(ProseTextFontColor, false),
												getFromColors(ProseTextFontColor, false),
													ui->advancedFontColorGroupBox);
	proseTextFontColor->setObjectName(QString::fromUtf8("proseTextFontColor"));
	ui->gridLayout_9->addWidget(proseTextFontColor, 4, 1, 1, 1);
}

Settings::~Settings()
{
	delete ui;
}

void Settings::addActionToToolbarTable()
{
	QList<QTableWidgetItem *> allActionItemList = ui->tableWidgetAllActions->selectedItems();
	if (allActionItemList.isEmpty())
		return;
	QList<QTableWidgetItem *> toolbarItemList = ui->tableWidgetToolBarActions->selectedItems();
	int currentRowInToolbarAction = ui->tableWidgetToolBarActions->rowCount();
	if (!toolbarItemList.isEmpty())
		currentRowInToolbarAction = toolbarItemList.at(0)->row()+1;

	int currentRowInAllAction = allActionItemList.at(0)->row();
	QTableWidgetItem *item = new QTableWidgetItem(*allActionItemList.at(0));
	if (!item) return;

	ui->tableWidgetToolBarActions->insertRow(currentRowInToolbarAction);
	ui->tableWidgetToolBarActions->setItem( currentRowInToolbarAction, 0, item);
	if (!item->data(Qt::UserRole+1).toString().contains("separator",Qt::CaseInsensitive))
	{
		ui->tableWidgetAllActions->removeRow(currentRowInAllAction);
	}
}

void Settings::removeActionFromToolbarTable()
{
	QList<QTableWidgetItem *> toolbarItemList = ui->tableWidgetToolBarActions->selectedItems();
	if (toolbarItemList.isEmpty())
		return;

	int currentRowInToolbarAction = toolbarItemList.at(0)->row();

	QTableWidgetItem *item = new QTableWidgetItem(*toolbarItemList.at(0));
	if (!item) return;

	if (!item->data(Qt::UserRole+1).toString().contains("separator",Qt::CaseInsensitive) )
	{
		ui->tableWidgetAllActions->insertRow(ui->tableWidgetAllActions->rowCount());
		ui->tableWidgetAllActions->setItem( ui->tableWidgetAllActions->rowCount()-1, 0, item);
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
	QList<QTableWidgetItem *> itemList = ui->tableWidgetToolBarActions->selectedItems();
	if (itemList.isEmpty())
		return;
	int row = itemList.at(0)->row();
	QTableWidgetItem *neighborItem = ui->tableWidgetToolBarActions->item( row+neighbor, 0);
	if (neighborItem)
	{
		QTableWidgetItem *firstItem = new  QTableWidgetItem(*itemList.at(0));
		QTableWidgetItem *secondItem = new  QTableWidgetItem(*neighborItem);
		ui->tableWidgetToolBarActions->setItem(row+neighbor, 0, firstItem);
		ui->tableWidgetToolBarActions->setItem(row, 0, secondItem);
		ui->tableWidgetToolBarActions->selectRow(row+neighbor);
		ui->tableWidgetToolBarActions->update();
	}
}

void Settings::initializeActionTables(const QMap<QString, QAction *> &actionsMap, const QStringList &toolBarItems)
{
	//table for all actions
	ui->tableWidgetAllActions->setRowCount(1);

	QTableWidgetItem *item = new QTableWidgetItem("------"+tr("Separator")+"------");
	item->setData(Qt::UserRole+1, "separator");
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setTextAlignment(Qt::AlignCenter);
	ui->tableWidgetAllActions->setItem(0, 0, item);
	if (!actionsMap.isEmpty())
	{
		QMap<QString, QAction *>::const_iterator it = actionsMap.constBegin();
		int index = 1;
		while (it != actionsMap.constEnd())
		{
			if ( toolBarItems.contains(it.key(), Qt::CaseInsensitive) )
			{
				++it;
				continue;
			}
			QString text=it.value()->text();
			text.remove("&");
			item = new QTableWidgetItem(text);
			item->setData(Qt::UserRole+1, it.key());
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
	for (int i=0; i < toolBarItems.size(); ++i)
	{
		QString  actionString = toolBarItems.at(i);
		if (actionString.contains("separator", Qt::CaseInsensitive))
		{
			item = new QTableWidgetItem("------"+tr("Separator")+"------");
			item->setData(Qt::UserRole+1, "separator");
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setTextAlignment(Qt::AlignCenter);
		}
		else
		{
			QAction *action = actionsMap.value(actionString);
			if (action)
			{
				QString text = action->text();
				text.remove("&");
				item = new QTableWidgetItem(text);
				item->setData(Qt::UserRole+1, actionString);
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				item->setIcon(action->icon().isNull() ? QIcon(":/resources/images/empty.png") : action->icon());
			}
			else
			{
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
	QList<QTableWidgetItem *> itemList = ui->tableWidgetToolBarActions->selectedItems();
	int row = 0;
	if (!itemList.isEmpty())
		row = itemList.at(0)->row();
	ui->pushButtonActionAdd->setEnabled(false);
	ui->pushButtonActionRemove->setEnabled(true);
	if (row == 0)
		ui->pushButtonActionTop->setEnabled(false);
	else
		ui->pushButtonActionTop->setEnabled(true);

	if (row == ui->tableWidgetToolBarActions->rowCount()-1)
		ui->pushButtonActionBottom->setEnabled(false);
	else
		ui->pushButtonActionBottom->setEnabled(true);
}

void Settings::tableAllActionsEntered()
{
	ui->pushButtonActionAdd->setEnabled(true);
	ui->pushButtonActionRemove->setEnabled(false);
	ui->pushButtonActionTop->setEnabled(false);
	ui->pushButtonActionBottom->setEnabled(false);
}

void Settings::getColorForPushButton()
{
	QPushButton *colorPushButton = qobject_cast<QPushButton *>(sender());
	QColor color = QColorDialog::getColor(colorPushButton->palette().background().color(), this);
	if (color.isValid()) 
		{
		colorPushButton->setPalette(QPalette(color));
		colorPushButton->setAutoFillBackground(true);
		}	
}

void Settings::browseForDataBasePath()
{
	QString dir = QFileDialog::getExistingDirectory(this,tr("Add Path For Data Base"), ui->lineEditDataBasePath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if ( !dir.isEmpty() ) 
	{
		dir.replace(QString("\\"),QString("/"));
		if ( !dir.endsWith('/') ) dir+="/";
		QString currentPath = ui->lineEditDataBasePath->text();
		if (!currentPath.isEmpty())
			currentPath+=";";
//		ui->lineEditDataBasePath->setText(currentPath+dir);
		ui->lineEditDataBasePath->setText(dir);//in this version Saaghar just use its first search path
	}
}

void Settings::browseForBackground()
{
	QList<QByteArray> formatList = QImageReader::supportedImageFormats();
	QString imageFormats = "";
	for (int i = 0; i<formatList.size(); ++i)
	{
		QString tmp = QString(formatList.at(i));
		if (!tmp.isEmpty())
			imageFormats+= "*."+QString(formatList.at(i));
		if (i<formatList.size()-1)
			imageFormats+=" ";
	}

	QString location=QFileDialog::getOpenFileName(this,tr("Browse Background Image"), ui->lineEditBackground->text(),"Image files ("+imageFormats+")");//*.png *.bmp)");
	if ( !location.isEmpty() ) 
		{
		location.replace(QString("\\"),QString("/"));
		ui->lineEditBackground->setText(location);
		}
}

void Settings::browseForIconTheme()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Browse Theme Directory"), ui->lineEditIconTheme->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if ( !dir.isEmpty() ) 
		{
		dir.replace(QString("\\"),QString("/"));
		if ( !dir.endsWith('/') ) dir+="/";
		ui->lineEditIconTheme->setText(dir);
		}
}

void Settings::setUnCheckedOtherFontColorGroupBox(bool unChecked)
{
	if (sender() == ui->advancedFontColorGroupBox)
		ui->globalFontColorGroupBox->setChecked(!unChecked);
	else if (sender() == ui->globalFontColorGroupBox)
		ui->advancedFontColorGroupBox->setChecked(!unChecked);
}

/* static */
QFont Settings::getFromFonts(FontColorItem type, bool canLoadDefault)
{
	QFont font;
	if (type == OutLineFontColor)
		canLoadDefault = false;
	if (Settings::READ("Global Font", false).toBool() && canLoadDefault)
	{
		font = Settings::hashFonts.value(QString::number(int(DefaultFontColor))).value<QFont>();
	}
	else
	{
		font = Settings::hashFonts.value(QString::number(int(type))).value<QFont>();
	}
	font.setStyleStrategy(QFont::PreferAntialias);
	return font;
}

/* static */
QColor Settings::getFromColors(FontColorItem type, bool canLoadDefault)
{
	QColor color;
	if (type == OutLineFontColor)
		canLoadDefault = false;
	if (Settings::READ("Global Font", false).toBool() && canLoadDefault)
	{
		color = Settings::hashColors.value(QString::number(int(DefaultFontColor))).value<QColor>();
	}
	else
	{
		color = Settings::hashColors.value(QString::number(int(type))).value<QColor>();
	}
	return color;
}

/* static */
void Settings::insertToFontColorHash(QHash<QString, QVariant> *hash, const QVariant &variant, FontColorItem type)
{
	if (!hash) return;
	//if (Settings::READ("Global Font", false).toBool() && type != DefaultFontColor) return;
	hash->insert(QString::number(int(type)), variant);
}


/*******************************
// class CustomizeRandomDialog
********************************/
CustomizeRandomDialog::CustomizeRandomDialog(QWidget *parent, bool openInNewTab) : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
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
	if (this->objectName().isEmpty())
		this->setObjectName(QString::fromUtf8("this"));
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
	this->setWindowTitle(QApplication::translate("CustomizeRandomDialog", "Customize Faal & Random", 0, QApplication::UnicodeUTF8));
	checkBox->setText(QApplication::translate("CustomizeRandomDialog", "Open in new Tab", 0, QApplication::UnicodeUTF8));
	label->setText(QApplication::translate("CustomizeRandomDialog", "Select Random Range:", 0, QApplication::UnicodeUTF8));
	pushButtonOk->setText(QApplication::translate("CustomizeRandomDialog", "Ok", 0, QApplication::UnicodeUTF8));
	pushButtonCancel->setText(QApplication::translate("CustomizeRandomDialog", "Cancel", 0, QApplication::UnicodeUTF8));
}


void CustomizeRandomDialog::acceptSettings(bool *checked)
{
	*checked = checkBox->isChecked();
}

/*******************************
// class FontColorSelector
********************************/

#include <QFontDialog>

FontColorSelector::FontColorSelector(const QFont &defaultFont, const QColor &defaultColor, QWidget *parent, const QString &sampleText)
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
	colorSelector->setFixedSize(24,24);
	QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	connect(fontSelector, SIGNAL(clicked()), this, SLOT(selectFont()));
	connect(colorSelector, SIGNAL(clicked()), this, SLOT(selectColor()));

	//bold = new QCheckBox("Bold", this);
	//italic = new QCheckBox("Italic", this);

	setSampleFont(defaultFont);

	if (defaultColor.isValid())
		setColor(defaultColor);
	else
		setColor(color());

	hLayout = new QHBoxLayout(0);
	hLayout->setSpacing(1);
	hLayout->setContentsMargins(1,1,1,1);
	hLayout->addWidget(sampleLabel);
	hLayout->addItem(horizontalSpacer);
	hLayout->addWidget(fontInfo);
	hLayout->addWidget(colorSelector);
	hLayout->addWidget(fontSelector);
	//hLayout->addWidget(bold);
	//hLayout->addWidget(italic);

	this->setLayout(hLayout);
}

QColor FontColorSelector::color()
{
	return sampleLabel->palette().color(QPalette::WindowText);
}

void FontColorSelector::setColor(const QColor &color)
{
	if (!color.isValid()) return;

	colorSelector->setPalette(QPalette(color));
	colorSelector->setAutoFillBackground(true);

	QPalette p(sampleLabel->palette());
	p.setColor(QPalette::WindowText, color);
	sampleLabel->setPalette(p);
}

void FontColorSelector::setSampleFont(const QFont &font)
{
	sampleLabel->setFont(font);
	
	fontInfo->setText(" "+font.family()+", "+QString::number(font.pointSize())+" ");
	QFont fontInfoLabelFont(fontInfo->font());
	fontInfoLabelFont.setBold(font.bold());
	fontInfoLabelFont.setItalic(font.italic());
	fontInfo->setFont(fontInfoLabelFont);
}

void FontColorSelector::selectFont()
{
	//bool ok;
	QFont tmp = sampleFont();
	//QFont font = QFontDialog::getFont(&ok, sampleFont(), this);
	QFontDialog fontDialog(tmp, this);
	connect(&fontDialog, SIGNAL(currentFontChanged(QFont)), this, SLOT(setSampleFont(QFont)));
	//connect(&fontDialog, SIGNAL(currentFontChanged(QFont)), this, SLOT(setSampleFont(QFont)));

	if (fontDialog.exec())
	{
		setSampleFont(fontDialog.selectedFont());
	}
	else
	{
		//maybe after emmitting signals font has been changed
		setSampleFont(tmp);
	}
}

void FontColorSelector::selectColor()
{
	QColor color = QColorDialog::getColor(colorSelector->palette().background().color(), this);
	setColor(color);
}
