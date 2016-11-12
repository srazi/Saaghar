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

#if QT_VERSION >= 0x050000
protected:
    void paintEvent(QPaintEvent* event);
#endif

signals:
    void resultsRefreshRequired();
};

#endif // SEARCHOPTIONSDIALOG_H
