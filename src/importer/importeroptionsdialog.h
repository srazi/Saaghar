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
    explicit ImporterOptionsDialog(QWidget *parent = 0);
    ~ImporterOptionsDialog();

public slots:
    void doImportPreview();
    void doSaveImport();

private slots:
    void doLoadFile();
    void currentTabChanged();
    void contentChanghed();

private:
    void reset();
    void init(const QString &content, const QString &type = QLatin1String("txt"));
    void setDisableElements(bool disable);

    Ui::ImporterOptionsDialog *ui;
    ImporterInterface* m_importer;

    QString m_content;
    bool m_contentViewInDirtyState;
};

#endif // IMPORTEROPTIONSDIALOG_H
