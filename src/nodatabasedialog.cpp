/***************************************************************************
 *  This file is part of Saaghar, a Persian poetry software                *
 *                                                                         *
 *  Copyright (C) 2010-2016 by S. Razi Alavizadeh                          *
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

#include "nodatabasedialog.h"

#include <QTimer>

NoDataBaseDialog::NoDataBaseDialog(QWidget* parent, Qt::WindowFlags f) :
    QDialog(parent, f),
    ui(new Ui::NoDataBaseDialog)
{
    ui->setupUi(this);
    _clickedButton = ui->exitPushButton;
    connect(ui->exitPushButton, SIGNAL(clicked(bool)), this, SLOT(buttonCheckStateToggled()));
    connect(ui->selectDataBase, SIGNAL(clicked(bool)), this, SLOT(buttonCheckStateToggled()));
    connect(ui->createDataBaseFromLocal, SIGNAL(clicked(bool)), this, SLOT(buttonCheckStateToggled()));
    connect(ui->createDataBaseFromRemote, SIGNAL(clicked(bool)), this, SLOT(buttonCheckStateToggled()));
}

NoDataBaseDialog::~NoDataBaseDialog()
{
    delete ui;
}

void NoDataBaseDialog::buttonCheckStateToggled()
{
    QPushButton* senderButton = qobject_cast<QPushButton*>(sender());
    if (senderButton) {
        _clickedButton = senderButton;
    }

    if (_clickedButton) {
        this->accept();
    }

}

QPushButton* NoDataBaseDialog::clickedButton()
{
    return _clickedButton;
}

void NoDataBaseDialog::adjustSizeSlot()
{
    adjustSize();
}

void NoDataBaseDialog::showEvent(QShowEvent* ev)
{
    QDialog::showEvent(ev);

    QTimer::singleShot(0, this, SLOT(adjustSizeSlot()));
}
