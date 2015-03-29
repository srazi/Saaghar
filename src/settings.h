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

#ifndef SETTINGS_H
#define SETTINGS_H

#if defined(Q_WS_X11) && !defined(Q_OS_X11)
#define Q_OS_X11
#endif

#define ICON_PATH Settings::currentIconThemePath()
#include "ui_settings.h"

#include <QMultiSelectWidget>
#include <QDialog>

namespace Ui
{
class Settings;
}

class SaagharWindow;
class FontColorSelector;

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(SaagharWindow* parent = 0);
    ~Settings();

    FontColorSelector* outlineFontColor;
    FontColorSelector* globalTextFontColor;
    FontColorSelector* poemTextFontColor;
    FontColorSelector* proseTextFontColor;
    FontColorSelector* sectionNameFontColor;
    FontColorSelector* titlesFontColor;
    FontColorSelector* numbersFontColor;
    FontColorSelector* backgroundFontColor;
    FontColorSelector* matchedFontColor;

    Ui::Settings* ui;
    void initializeActionTables(const QMap<QString, QAction*> &actionsMap, const QStringList &toolBarItems);

    enum FontColorItem {
        DefaultFontColor = 0,
        PoemTextFontColor = 1,
        ProseTextFontColor = 2,
        SectionNameFontColor = 3,
        TitlesFontColor = 4,
        NumbersFontColor = 5,
        OutLineFontColor = 6
    };

    static QHash<QString, QVariant> hashFonts;
    static QHash<QString, QVariant> hashColors;
    static QFont getFromFonts(FontColorItem type = DefaultFontColor, bool canLoadDefault = true);
    static QColor getFromColors(FontColorItem type = DefaultFontColor, bool canLoadDefault = true);
    static void insertToFontColorHash(QHash<QString, QVariant>* hash, const QVariant &variant, FontColorItem type = DefaultFontColor);

    //settings variables
    static QVariant READ(const QString &key, const QVariant &defaultValue = QVariant());
    static void WRITE(const QString &key, const QVariant &value = QVariant());

    inline static void LOAD_VARIABLES(const QHash<QString, QVariant> &variables) {
        Settings::VariablesHash.clear();
        Settings::VariablesHash = variables;
    }

    inline static QVariant GET_VARIABLES_VARIANT() {
        return QVariant(Settings::VariablesHash);
    }

    void applySettings();

    static QString currentIconThemePath();

public slots:
    void accept();

private slots:
    void setUnCheckedOtherFontColorGroupBox(bool unChecked);
    void removeActionFromToolbarTable();
    void addActionToToolbarTable();
    void topAction();
    void bottomAction();
    void tableAllActionsEntered();
    void tableToolBarActionsEntered();
    void browseForBackground();
    void browseForIconTheme();
    void browseForDataBasePath();

private:
    void replaceWithNeighbor(int neighbor);
    void setupTaskManagerUi();

    static QHash<QString, QVariant> VariablesHash;
    static QString s_currentIconPath;
};

class CustomizeRandomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CustomizeRandomDialog(QWidget* parent = 0, bool checked = false);
    ~CustomizeRandomDialog();
    void acceptSettings(bool* checked);
    QMultiSelectWidget* selectRandomRange;

private:
    QLabel* label;
    QGridLayout* gridLayout;
    QVBoxLayout* verticalLayout;
    QCheckBox* checkBox;
    QHBoxLayout* horizontalLayout;
    QSpacerItem* horizontalSpacer;
    QPushButton* pushButtonOk;
    QPushButton* pushButtonCancel;
    void setupui();
    void retranslateUi();
};


class FontColorSelector : public QWidget
{
    Q_OBJECT

public:
    FontColorSelector(const QFont &defaultFont = QFont(), const QColor &defaultColor = QColor(), QWidget* parent = 0, const QString &sampleText = "");
    inline QFont sampleFont() {return sampleLabel->font();}
    QColor color();
    void setColorSelector(bool colorSelector);

public slots:
    void setSampleFont(const QFont &font);
    void setColor(const QColor &color);
    void enableAll(bool enabled);
    void disableAll(bool disable);

private:
    QLabel* fontInfo;
    QLabel* sampleLabel;
    QPushButton* fontSelector;
    QPushButton* colorSelector;
    QCheckBox* bold;
    QCheckBox* italic;
    QHBoxLayout* hLayout;
    QColor m_color;

private slots:
    void selectFont();
    void selectColor();
};

#endif // SETTINGS_H
