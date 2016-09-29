#include "selectcreatedialog.h"
#include "ui_selectcreatedialog.h"
#include "saagharapplication.h"
#include "outlinemodel.h"

SelectCreateDialog::SelectCreateDialog(QWidget* parent, QList<int> path) :
    QDialog(parent),
    ui(new Ui::SelectCreateDialog)
{
    ui->setupUi(this);
    ui->lineEdit->setDisabled(true);

    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setModel(sApp->outlineModel());
    ui->treeView->setRootIndex(sApp->outlineModel()->indexFromPath(path));

    connect(ui->createRadioButton, SIGNAL(toggled(bool)), ui->lineEdit, SLOT(setEnabled(bool)));
    connect(ui->selectRadioButton, SIGNAL(toggled(bool)), ui->treeView, SLOT(setEnabled(bool)));
}

SelectCreateDialog::~SelectCreateDialog()
{
    delete ui;
}

QString SelectCreateDialog::newTitle() const
{
    return ui->lineEdit->text().simplified();
}

QList<int> SelectCreateDialog::selectedPath() const
{
    return ui->treeView->selectionModel()->selectedIndexes().isEmpty()
            ? QList<int>()
            : sApp->outlineModel()->pathFromIndex(ui->treeView->selectionModel()->selectedIndexes().at(0));
}

QList<GanjoorCat> SelectCreateDialog::selectedCatPath(bool reversed) const
{
    return ui->treeView->selectionModel()->selectedIndexes().isEmpty()
            ? QList<GanjoorCat>()
            : sApp->outlineModel()->catPathFromIndex(ui->treeView->selectionModel()->selectedIndexes().at(0), reversed);
}

GanjoorCat SelectCreateDialog::selectedCat() const
{
    return ui->treeView->selectionModel()->selectedIndexes().isEmpty()
            ? GanjoorCat()
            : ui->treeView->selectionModel()->selectedIndexes().at(0).data(OutlineModel::CategoryRole).value<GanjoorCat>();
}

void SelectCreateDialog::setForceCreate(bool force)
{
    ui->createRadioButton->show();
    ui->lineEdit->show();
    ui->selectRadioButton->setVisible(!force);
    ui->treeView->setVisible(!force);

    if (force) {
        ui->createRadioButton->setChecked(true);
    }
}

void SelectCreateDialog::setForceSelect(bool force)
{
    ui->selectRadioButton->show();
    ui->treeView->show();
    ui->createRadioButton->setVisible(!force);
    ui->lineEdit->setVisible(!force);

    if (force) {
        ui->selectRadioButton->setChecked(true);
    }
}

bool SelectCreateDialog::createNewCat() const
{
    return ui->createRadioButton->isChecked();
}
