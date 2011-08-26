/***************************************************************************
 *  This file is part of QMultiSelectWidget, a custom Qt widget            *
 *                                                                         *
 *  Copyright (C) 2011 by S. Razi Alavizadeh                               *
 *  E-Mail: <s.r.alavizadeh@gmail.com>, WWW: <http://pojh.iBlogger.org>    *
 *                                                                         *
 *  There is some code from qtcenter forum & QxtCheckComboBox.from libqxt..*
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

#include "QMultiSelectWidget.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QKeyEvent>
#include<QDebug>

const int LAST_CHECK_STATE = Qt::UserRole+10;

CustomComboBox::CustomComboBox(QWidget* parent, QListWidget *viewWidget, QObject *filterObject) : QComboBox(parent)
{
	containerMousePress = false;

	if (viewWidget && viewWidget->model())
	{
		setModel(viewWidget->model());
		setView(viewWidget);
	}

	//read-only contents
	QLineEdit* lineEdit = new QLineEdit(this);
//#if QT_VERSION > 0x040700
//	lineEdit->setPlaceholderText(tr("Select Search Scope..."));
//#endif
	lineEdit->setReadOnly(true);
	setLineEdit(lineEdit);
	lineEdit->disconnect(this);

	setSizeAdjustPolicy(QComboBox::AdjustToContents);

	setInsertPolicy(QComboBox::NoInsert);

	installEventFilter(filterObject);
	view()->installEventFilter(filterObject);
	view()->window()->installEventFilter(filterObject);
	view()->viewport()->installEventFilter(filterObject);
}

CustomComboBox::~CustomComboBox()
{
}

/*!
	\reimp
 */
void CustomComboBox::wheelEvent( QWheelEvent * e )
{
	e->ignore();
}

/*!
	\reimp
 */
void CustomComboBox::hidePopup()
{
	if (containerMousePress)
		QComboBox::hidePopup();
}

QMultiSelectWidget::QMultiSelectWidget(QWidget* parent, QMultiSelectWidget::WidgetType type)
	: QListWidget(parent)
{
	separator = " - ";
	//setMaximumHeight(180);
	overCheckBox = false;
	//containerMousePress = false;
	//listWidget = this;//qobject_cast<QListWidget *>(this);
	//listWidget = new QListWidget(parent);
	if (type == QMultiSelectWidget::ComboBoxView)
	{
		comboWidget = new CustomComboBox(parent, this/*qobject_cast<QListWidget *>(this)*/, this);
		//comboWidget->view()->setMaximumHeight(200);
		comboWidget->view()->window()->setMaximumHeight(300);
		//comboWidget->view()->viewport()->setMaximumHeight(200);
	}
	else
		comboWidget = 0;

	//connect(this, SIGNAL(itemCheckStateChanged(QListWidgetItem*)), this, SLOT(checkStateChanged(QListWidgetItem*)));
	connect(this, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemDataChanged(QListWidgetItem*)));
	connect(this, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(clickedOnItem(QListWidgetItem*)));
	connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(doubleClickedOnItem(QListWidgetItem*)));
	//init();
	updateSelectedLists();
}

QMultiSelectWidget::~QMultiSelectWidget()
{
}

// returns the selected item whose checkbox lies under 'pos'
// returns 0 if not selected, no item at pos, or does not fit inside checkbox
bool QMultiSelectWidget::isCheckBoxUnderPosition(const QPoint& pos)
{
	QListWidgetItem* item = itemAt(pos);
	if (item)
	{
		// with the help of styles, check if checkbox rect contains 'pos'
		QStyleOptionButton opt;
		opt.QStyleOption::operator=(viewOptions());
		opt.rect = visualItemRect(item);
		QRect r = style()->subElementRect(QStyle::SE_ViewItemCheckIndicator, &opt);
		// assure that the item is also selected
		if (/*selectedItems().contains(item) && */r.contains(pos))
		{
			return true;//item;
		}
	}
	return false;
}

void QMultiSelectWidget::disableOtherItems(QListWidgetItem *excludedItem)
{
	for (int i=0; i<count(); ++i)
	{
		if (item(i) != excludedItem)
			item(i)->setFlags(item(i)->flags()^Qt::ItemIsEnabled); //->setHidden(true);
	}
}

void QMultiSelectWidget::enableOtherItems(QListWidgetItem *excludedItem)
{
	for (int i=0; i<count(); ++i)
	{
		if (item(i) != excludedItem)
			item(i)->setFlags(item(i)->flags()|Qt::ItemIsEnabled);//setHidden(false);
	}
}

void QMultiSelectWidget::itemDataChanged(QListWidgetItem *item)
{
	disconnect(this, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemDataChanged(QListWidgetItem*)));
	if (item->checkState() != item->data(LAST_CHECK_STATE))
	{
		item->setData(LAST_CHECK_STATE, item->checkState());
		checkStateChanged(item);
		emit itemCheckStateChanged(item);
	}
	connect(this, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemDataChanged(QListWidgetItem*)));
}

void QMultiSelectWidget::checkStateChanged(QListWidgetItem *item)
{
	if (noMultiSelectItems.contains(item))
	{
		if (item->checkState() == Qt::Checked)
			disableOtherItems(item);
		else
		{
			enableOtherItems(item);
			if (comboWidget)
			{
				comboWidget->setCurrentIndex(-1);
				comboWidget->lineEdit()->setText("");
			}
		}
	}
	else
	{
		QListWidgetItem *parentItem = parentChildHash.value(item);
		
		/***
		//compute check state of parent and children
		***/
		//if item is a child
		if (parentItem)
		{
			QList<QListWidgetItem *> childrenList = parentChildHash.keys(parentItem);
			int numOfChecked = 0;
			for (int i = 0; i<childrenList.size(); ++i)
			{
				if ( childrenList.at(i)->checkState() == Qt::Checked )
					++numOfChecked;
			}
			if (numOfChecked == 0)
			{
				parentItem->setCheckState(Qt::Unchecked);
				parentItem->setData(LAST_CHECK_STATE, Qt::Unchecked);
			}
			else if (numOfChecked < childrenList.size())
			{
				parentItem->setCheckState(Qt::PartiallyChecked);
				parentItem->setData(LAST_CHECK_STATE, Qt::PartiallyChecked);
			}
			else if (numOfChecked == childrenList.size())
			{
				parentItem->setCheckState(Qt::Checked);
				parentItem->setData(LAST_CHECK_STATE, Qt::Checked);
			}
		}
		
		//if item is a parent
		QList<QListWidgetItem *> childrenList = parentChildHash.keys(item);
		for (int i = 0; i<childrenList.size(); ++i)
		{
			childrenList.at(i)->setCheckState(item->checkState());
			childrenList.at(i)->setData(LAST_CHECK_STATE, item->checkState());
		}
	}

	updateSelectedLists();
}


void QMultiSelectWidget::setSeparator(const QString &sep)
{
	separator = sep;
	updateToolTips();
}

QList<QListWidgetItem *> QMultiSelectWidget::getSelectedItemList()
{
	return selectedItemsList;
}

QStringList QMultiSelectWidget::getSelectedStringList()
{
	return selectedList;
}

QString QMultiSelectWidget::getSelectedString()
{
	return selectedStr;
}

void QMultiSelectWidget::updateSelectedLists()
{
	//QList<QListWidgetItem *> parents = parentChildHash.values();
	//QList<QListWidgetItem *> childrenSkipList;
	selectedList.clear();
	selectedItemsList.clear();

	for (int i=0; i<count(); ++i)
	{
		QListWidgetItem *iItem = item(i);
		
		if ( (iItem->flags() & Qt::ItemIsEnabled) && iItem->checkState() == Qt::Checked)
		{
//			if (parents.contains(iItem))
//			{//iItem is a parent and we should skip all of its children.
//				childrenSkipList << parentChildHash.keys(iItem);
//			}
			QListWidgetItem *parentItem = parentChildHash.value(iItem);	
			if (!(parentItem && parentItem->checkState() == Qt::Checked))
			{
				selectedList << iItem->text();
				selectedItemsList << iItem;
			}
		}
	}

	updateToolTips();
}

void QMultiSelectWidget::updateToolTips()
{
	if (selectedList.isEmpty())
		selectedStr = tr("Selected Items: no item!");
	else
		selectedStr = tr("Selected Items:< /br>")+selectedList.join(separator);

	QString dirStr = "ltr";
	if	(layoutDirection() == Qt::RightToLeft)
		dirStr = "rtl";
	qDebug() << "dirStr=" << dirStr;
	setToolTip(QString("<p dir=\'%1\'>").arg(dirStr)+selectedStr+"</p>");
	if (comboWidget)
	{
		comboWidget->setToolTip(QString("<p dir=\'%1\'>").arg(dirStr)+selectedStr+"</p>");
		comboWidget->lineEdit()->setText(selectedList.join("-"));
	}
}

void QMultiSelectWidget::clickedOnItem(QListWidgetItem *item)
{
//qDebug() << "clickedOnItem";
/*QRect rect = visualItemRect(item);
QStyle *style = style();
QRect checkRect = style->subElementRect(QStyle::SE_CheckBoxContents, item);*/
//qDebug() << "item->flags()=" << (int)item->flags();
//qDebug() << "item->flags()&=" << (int)(item->flags() & Qt::ItemIsUserCheckable);
	if ( (item->flags() & Qt::ItemIsUserCheckable) && (item->flags() & Qt::ItemIsEnabled) && !overCheckBox)
		item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
	else if (!(item->flags() & Qt::ItemIsUserCheckable)  && (item->flags() & Qt::ItemIsEnabled))
	{qDebug() <<  "NONE-overCheckBox2=";
		setCurrentItem(item);
		if (comboWidget)
			comboWidget->QComboBox::hidePopup();
	}
//comboWidget->containerMousePress = false;
}

void QMultiSelectWidget::doubleClickedOnItem(QListWidgetItem *item)
{
//comboWidget->hidePopup();
//	if ( (item->flags() & Qt::ItemIsUserCheckable) && !overCheckBox)
//		item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);

//	if (comboWidget)
//		comboWidget->QComboBox::hidePopup();
}

QListWidgetItem *QMultiSelectWidget::insertRow(int row, const QString &label, bool checkable,
										const QString &data, Qt::ItemDataRole role, bool monoSelection, QListWidgetItem *parent)
{
	QListWidgetItem *newItem = new QListWidgetItem(label);
//	//just for test!!
//	if(row == 0) checkable = false;
//	if(row == 1) monoSelection = true;
	if (checkable)
	{
		newItem->setCheckState(Qt::Unchecked);//Qt::ItemIsEnabled
		newItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
		newItem->setData(LAST_CHECK_STATE, newItem->checkState());
	}
	else
	{
		newItem->setFlags(Qt::ItemIsEnabled/*|Qt::ItemIsSelectable*/);
	}
	
	if (monoSelection)
		insertNoMultiSelectItem(newItem);

//if (row>2 && row <6)//for test parent is replaced
//{parent = item(2);}

	if (parent && parent != newItem)
	{
		parentChildHash.insert(newItem, parent);
	}

	if (!data.isEmpty())
		newItem->setData(role, data);

	insertItem(row, newItem);
//if (row>10)
//{
//newItem->setHidden(true);
//}
	return newItem;
}

void QMultiSelectWidget::insertNoMultiSelectItem(QListWidgetItem *item)
{
	if (item)
		noMultiSelectItems << item;
}

//QWidget *QMultiSelectWidget::getWidgetInstance()
//{
//	QGridLayout *gridLayout = new QGridLayout();
//	containerWidget = new QWidget();
//	setParent(containerWidget);
////	if (comboWidget)
////	{
////		comboWidget->setView(0);
////		comboWidget->setModel(0);
////	}

//	gridLayout->addWidget(this);
//	containerWidget->setLayout(gridLayout);
//	return containerWidget;
//}

//QListWidget *QMultiSelectWidget::getListWidgetInstance()
//{
//	return qobject_cast<QListWidget *>(this);
//}

QComboBox *QMultiSelectWidget::getComboWidgetInstance()
{
	//maybe 0
	return comboWidget;
}

void QMultiSelectWidget::init()
{
	//listWidget->hide();
	//setModel(listWidget->model());
	//setView(listWidget);
	//privateObject = new QxtCheckComboBoxPrivate(this);
	//checkModel = new QxtCheckComboModel(this);
	//setModel(checkModel);
/////////////////////////////////
//	connect(this, SIGNAL(activated(int)), privateObject, SLOT(toggleCheckState(int)));
//	connect(checkModel/*model()*/, SIGNAL(checkStateChanged()), privateObject, SLOT(updateCheckedItems()));
//	connect(checkModel/*model()*/, SIGNAL(rowsInserted(const QModelIndex &, int, int)), privateObject, SLOT(updateCheckedItems()));
//	connect(checkModel/*model()*/, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), privateObject, SLOT(updateCheckedItems()));
	
//	// read-only contents
//	QLineEdit* lineEdit = new QLineEdit(this);
//	lineEdit->setReadOnly(true);
//	setLineEdit(lineEdit);
//	lineEdit->disconnect(this);
//	setInsertPolicy(QComboBox::NoInsert);
	
	//view()->installEventFilter(privateObject);
	//view()->window()->installEventFilter(privateObject);
	//view()->viewport()->installEventFilter(privateObject);
	//this->installEventFilter(privateObject);
}

bool QMultiSelectWidget::eventFilter(QObject* receiver, QEvent* event)
{
	switch (event->type())
	{
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (receiver == comboWidget && (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down))
		{
			comboWidget->showPopup();
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Enter ||
				 keyEvent->key() == Qt::Key_Return ||
				 keyEvent->key() == Qt::Key_Escape)
		{
			// it is important to call QComboBox implementation
			comboWidget->QComboBox::hidePopup();
			if (keyEvent->key() != Qt::Key_Escape)
				return true;
		}
	}
	case QEvent::MouseButtonPress:
	{
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		overCheckBox = isCheckBoxUnderPosition(mouseEvent->pos());
qDebug() << "overCheckBox1=" << overCheckBox;
		comboWidget->containerMousePress = (receiver == window());
	//qDebug() << "eventFilter-MouseButtonPress|bool=" << comboWidget->containerMousePress << receiver->objectName();
		break;
	}
	case QEvent::MouseButtonRelease:
//	qDebug() << "eventFilter-MouseButtonRelease";
		comboWidget->containerMousePress = false;
		return false;
		break;
	default:
		break;
	}
	return false;
}


//void QMultiSelectWidget::insertPoets(const QList<GanjoorPoet *> &poets)
//{
//	int insertIndex = count();
//	for(int i=0; i<poets.size(); ++i)
//	{
//		insertRow(i+insertIndex, poets.at(i)->_Name, true,
//				QString::number(poets.at(i)->_ID), Qt::UserRole);
//	}
//}
