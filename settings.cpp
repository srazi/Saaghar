/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2011 by S. Razi Alavizadeh                          *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://www.pojh.co.cc>       *
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
#include "ui_settings.h"
#include "SaagharWidget.h"
#include <QFontDatabase>
#include <QColorDialog>
#include <QFileDialog>

Settings::Settings(QWidget *parent,	bool iconThemeState, QString iconThemePath, QMap<QString, QAction *> *actionsMap, QStringList toolBarItems ) :
	QDialog(parent),
	ui(new Ui::Settings)
{
	ui->setupUi(this);

	if (QApplication::layoutDirection() == Qt::RightToLeft)
	{
		ui->pushButtonActionAdd->setIcon(QIcon(":/resources/images/left.png"));
		ui->pushButtonActionRemove->setIcon(QIcon(":/resources/images/right.png"));
	}

	//in this version, they're not used.
	ui->pushButtonDataBasePath->hide();
	ui->lineEditDataBasePath->hide();
	ui->labelDataBasePath->hide();

	//font
	QFontDatabase fontDb;
	ui->comboBoxFontFamily->addItems( fontDb.families() );
	ui->comboBoxFontFamily->setCurrentIndex(ui->comboBoxFontFamily->findText(SaagharWidget::tableFont.family(), Qt::MatchExactly));
	ui->spinBoxFontSize->setValue(SaagharWidget::tableFont.pointSize());
	
	ui->checkBoxBeytNumbers->setChecked(SaagharWidget::showBeytNumbers);

	ui->checkBoxBackground->setChecked(SaagharWidget::backgroundImageState);
	ui->lineEditBackground->setText(SaagharWidget::backgroundImagePath);
	ui->lineEditBackground->setEnabled(SaagharWidget::backgroundImageState);
	ui->pushButtonBackground->setEnabled(SaagharWidget::backgroundImageState);
	connect( ui->pushButtonBackground, SIGNAL(clicked()), this, SLOT(browseForBackground()));	
	
	ui->checkBoxIconTheme->setChecked(iconThemeState);
	ui->lineEditIconTheme->setText(iconThemePath);
	ui->lineEditIconTheme->setEnabled(iconThemeState);
	ui->pushButtonIconTheme->setEnabled(iconThemeState);
	connect( ui->pushButtonIconTheme, SIGNAL(clicked()), this, SLOT(browseForIconTheme()));

	//colors
	connect( ui->pushButtonBackgroundColor, SIGNAL(clicked()), this, SLOT(getColorForPushButton()));
	connect( ui->pushButtonMatchedTextColor, SIGNAL(clicked()), this, SLOT(getColorForPushButton()));
	connect( ui->pushButtonTextColor, SIGNAL(clicked()), this, SLOT(getColorForPushButton()));
	ui->pushButtonBackgroundColor->setPalette(QPalette(SaagharWidget::backgroundColor));
	ui->pushButtonBackgroundColor->setAutoFillBackground(true);
	ui->pushButtonMatchedTextColor->setPalette(QPalette(SaagharWidget::matchedTextColor));
	ui->pushButtonMatchedTextColor->setAutoFillBackground(true);
	ui->pushButtonTextColor->setPalette(QPalette(SaagharWidget::textColor));
	ui->pushButtonTextColor->setAutoFillBackground(true);

	//initialize Action's Tables
	initializeActionTables(actionsMap, toolBarItems);

	connect(ui->pushButtonActionBottom, SIGNAL(clicked()), this, SLOT(bottomAction()));
	connect(ui->pushButtonActionTop, SIGNAL(clicked()), this, SLOT(topAction()));
	connect(ui->pushButtonActionAdd, SIGNAL(clicked()), this, SLOT(addActionToToolbarTable()));
	connect(ui->pushButtonActionRemove, SIGNAL(clicked()), this, SLOT(removeActionFromToolbarTable()));
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

void Settings::initializeActionTables(QMap<QString, QAction *> *actionsMap, QStringList toolBarItems)
{
	//table for all actions
	ui->tableWidgetAllActions->setRowCount(1);
	/*int width = ui->tableWidgetAllActions->columnWidth(0);
	int dashWidth = ui->tableWidgetAllActions->fontMetrics().width("-");
	width-=ui->tableWidgetAllActions->fontMetrics().width(tr("Separator"));
	QString dashStr="";
	if (width>0)
	{
		int numberOfDash = width/dashWidth;
		numberOfDash/=2;
		for (int i=0; i < numberOfDash; ++i)
		{
			dashStr+="-";
		}
	}*/
	QTableWidgetItem *item = new QTableWidgetItem("------"+tr("Separator")+"------");
	item->setData(Qt::UserRole+1, "separator");
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setTextAlignment(Qt::AlignCenter);
	ui->tableWidgetAllActions->setItem(0, 0, item);
	if (actionsMap)
	{
		QMap<QString, QAction *>::const_iterator it = actionsMap->constBegin();
		int index = 1;
		while (it != actionsMap->constEnd())
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
			item->setIcon(it.value()->icon());
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
			QAction *action = actionsMap->value(actionString);
			if (action)
			{
				QString text = action->text();
				text.remove("&");
				item = new QTableWidgetItem(text);
				item->setData(Qt::UserRole+1, actionString);
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				item->setIcon(action->icon());
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

void Settings::browseForBackground()
{
	QString location=QFileDialog::getOpenFileName(this,tr("Browse Background Image"), ui->lineEditBackground->text(),"Image files (*.png *.bmp)");
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

void Settings::acceptSettings(bool *iconThemeState, QString *iconThemePath, QStringList *toolbarItemsList)
{
	//font
	QFont font(ui->comboBoxFontFamily->currentText(), ui->spinBoxFontSize->value());
	SaagharWidget::tableFont = font;
	
	SaagharWidget::showBeytNumbers = ui->checkBoxBeytNumbers->isChecked();

	SaagharWidget::backgroundImageState = ui->checkBoxBackground->isChecked();
	SaagharWidget::backgroundImagePath = ui->lineEditBackground->text();

	*iconThemeState = ui->checkBoxIconTheme->isChecked();
	*iconThemePath = ui->lineEditIconTheme->text();
	
	//colors
	SaagharWidget::backgroundColor = ui->pushButtonBackgroundColor->palette().background().color();
	SaagharWidget::textColor = ui->pushButtonTextColor->palette().background().color();
	if (ui->pushButtonMatchedTextColor->palette().background().color() != SaagharWidget::textColor)//they must be different.
		SaagharWidget::matchedTextColor = ui->pushButtonMatchedTextColor->palette().background().color();

	//toolbar items
	toolbarItemsList->clear();
	for (int i=0; i<ui->tableWidgetToolBarActions->rowCount(); ++i)
	{
		QTableWidgetItem *item = ui->tableWidgetToolBarActions->item(i, 0);
		if (item)
		{
			toolbarItemsList->append(item->data(Qt::UserRole+1).toString());
		}
	}
}
