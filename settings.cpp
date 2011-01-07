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

Settings::Settings(QWidget *parent,	bool iconThemeState, QString iconThemePath) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
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
}

Settings::~Settings()
{
    delete ui;
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

void Settings::acceptSettings(bool *iconThemeState, QString *iconThemePath)
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
}
