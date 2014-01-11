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

#ifndef NODATABASEDIALOG_H
#define NODATABASEDIALOG_H

#include "ui_nodatabasedialog.h"

#include <QDialog>

namespace Ui
{
class NoDataBaseDialog;
}

class NoDataBaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NoDataBaseDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~NoDataBaseDialog();
    Ui::NoDataBaseDialog* ui;

    QPushButton* clickedButton();

private:
    void showEvent(QShowEvent *ev);

    QPushButton* _clickedButton;

private slots:
    void adjustSizeSlot();
    void buttonCheckStateToggled();
};

#endif // NODATABASEDIALOG_H
