#ifndef SEARCHOPTIONSDIALOG_H
#define SEARCHOPTIONSDIALOG_H

#include <QDialog>

namespace Ui
{
class SearchOptionsDialog;
}

class SearchOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchOptionsDialog(QWidget* parent = 0);
    ~SearchOptionsDialog();

private:
    void accept();
    Ui::SearchOptionsDialog* ui;

signals:
    void resultsRefreshRequired();
};

#endif // SEARCHOPTIONSDIALOG_H
