#ifndef IMPORTEROPTIONSDIALOG_H
#define IMPORTEROPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class ImporterOptionsDialog;
}

class ImporterInterface;

class ImporterOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImporterOptionsDialog(ImporterInterface* importer, const QString &fileName, const QString &content, QWidget *parent = 0);
    ~ImporterOptionsDialog();

public slots:
    void doImportPreview();
    void doSaveImport();

private:
    Ui::ImporterOptionsDialog *ui;
    ImporterInterface* m_importer;

    QString m_content;
};

#endif // IMPORTEROPTIONSDIALOG_H
