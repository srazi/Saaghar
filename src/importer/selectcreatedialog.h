#ifndef SELECTCREATEDIALOG_H
#define SELECTCREATEDIALOG_H

#include <QDialog>
#include "databaseelements.h"

namespace Ui {
class SelectCreateDialog;
}

class SelectCreateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectCreateDialog(QWidget* parent = 0, QList<int> path = QList<int>());
    ~SelectCreateDialog();

    QString newTitle() const;
    GanjoorCat selectedCat() const;
    QList<int> selectedPath() const;
    QStringList selectedTitlePath() const;

    void setForceCreate(bool force);
    void setForceSelect(bool force);

    bool createNewCat() const;

private:
    Ui::SelectCreateDialog *ui;
};

#endif // SELECTCREATEDIALOG_H
