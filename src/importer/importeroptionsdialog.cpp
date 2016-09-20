#include "importeroptionsdialog.h"
#include "ui_importeroptionsdialog.h"
#include "importer_interface.h"
#include "importermanager.h"

ImporterOptionsDialog::ImporterOptionsDialog(ImporterInterface* importer, const QString &fileName, const QString &content, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImporterOptionsDialog),
    m_importer(importer),
    m_content(content)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    ui->setupUi(this);

    ui->fileNameLineEdit->setText(fileName);
    ui->textEdit->setText(m_content);
    ui->textEdit->setReadOnly(true);
    ui->poemTitleLineEdit->setText(m_importer->options().poemStartPattern);

    ui->normalTextCheckBox->setChecked(m_importer->options().contentTypes & ImporterInterface::Options::NormalText);
    ui->classicCheckBox->setChecked(m_importer->options().contentTypes & ImporterInterface::Options::Poem);
    ui->whiteCheckBox->setChecked(m_importer->options().contentTypes & ImporterInterface::Options::WhitePoem);

    ui->importPushButton->setDisabled(true);

    connect(ui->previewPushButton, SIGNAL(clicked(bool)), this, SLOT(doImportPreview()));
    connect(ui->importPushButton, SIGNAL(clicked(bool)), this, SLOT(doSaveImport()));
    connect(ui->cancelPushButton, SIGNAL(clicked(bool)), this, SLOT(reject()));
}

ImporterOptionsDialog::~ImporterOptionsDialog()
{
    delete ui;
}

void ImporterOptionsDialog::doImportPreview()
{   ImporterInterface::Options options;

    options.contentTypes = ImporterInterface::Options::Unknown;

    if (ui->normalTextCheckBox->isChecked()) {
        options.contentTypes |= ImporterInterface::Options::NormalText;
    }
    else if (ui->classicCheckBox->isChecked()) {
        options.contentTypes |= ImporterInterface::Options::Poem;
    }
    else if (ui->whiteCheckBox->isChecked()) {
        options.contentTypes |= ImporterInterface::Options::WhitePoem;
    }

    options.poemStartPattern = ui->poemTitleLineEdit->text();

    m_importer->setOptions(options);
    m_importer->import(m_content);
    ui->textEdit->setText(ImporterManager::instance()->convertTo(m_importer->importData(), ImporterManager::PlainText));

    //ui->importPushButton->setText(tr("Close"));
    ui->importPushButton->setDisabled(false);
//    disconnect(ui->importPushButton, SIGNAL(clicked(bool)), this, SLOT(doImport()));
    //    connect(ui->importPushButton, SIGNAL(clicked(bool)), this, SLOT(accept()));
}

void ImporterOptionsDialog::doSaveImport()
{
    QDialog::accept();
}
