#include "importeroptionsdialog.h"
#include "ui_importeroptionsdialog.h"
#include "importer_interface.h"
#include "importermanager.h"

#include <QFile>
#include <QFileInfo>
#include <QFileDialog>

ImporterOptionsDialog::ImporterOptionsDialog(QWidget *parent) : //ImporterInterface* importer, const QString &fileName, const QString &content, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImporterOptionsDialog),
    m_importer(0)
  //,  m_content(content)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    ui->setupUi(this);

    //ui->fileNameLineEdit->setText(fileName);
    //ui->contentTextEdit->setText(m_content);
    ui->contentTextEdit->setReadOnly(true);
    ui->previewTextEdit->setReadOnly(true);
    ui->previewHtmlEdit->setReadOnly(true);

    setDisableElements(true);
    ui->importPushButton->setDisabled(true);
//    ui->poemTitleLineEdit->setText(m_importer->options().poemStartPattern);

//    ui->normalTextCheckBox->setChecked(m_importer->options().contentTypes & ImporterInterface::Options::NormalText);
//    ui->classicCheckBox->setChecked(m_importer->options().contentTypes & ImporterInterface::Options::Poem);
//    ui->whiteCheckBox->setChecked(m_importer->options().contentTypes & ImporterInterface::Options::WhitePoem);


    connect(ui->browsePushButton, SIGNAL(clicked(bool)), this, SLOT(doLoadFile()));
    connect(ui->previewPushButton, SIGNAL(clicked(bool)), this, SLOT(doImportPreview()));
    connect(ui->importPushButton, SIGNAL(clicked(bool)), this, SLOT(doSaveImport()));
    connect(ui->cancelPushButton, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged()));

    ui->tabWidget->widget(1)->setProperty("type", QVariant(ImporterManager::PlainText));
    ui->tabWidget->widget(2)->setProperty("type", QVariant(ImporterManager::HtmlText));
    ui->tabWidget->widget(3)->setProperty("type", QVariant(ImporterManager::EditingText));
}

ImporterOptionsDialog::~ImporterOptionsDialog()
{
    delete ui;
}

void ImporterOptionsDialog::doImportPreview()
{
    if (m_content.isEmpty() || !m_importer) {
        return;
    }
    ui->tabWidget->setCurrentWidget(ui->contentTab);

    ImporterInterface::Options options;

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
    //ui->previewTextEdit->setText(ImporterManager::instance()->convertTo(m_importer->importData(), ImporterManager::PlainText));

    //ui->importPushButton->setText(tr("Close"));
    ui->importPushButton->setDisabled(false);
//    disconnect(ui->importPushButton, SIGNAL(clicked(bool)), this, SLOT(doImport()));
    //    connect(ui->importPushButton, SIGNAL(clicked(bool)), this, SLOT(accept()));


    //ui->contentTextEdit->setProperty("dirty", QVariant(true));
//    ui->previewTextEdit->setProperty("dirty", QVariant(true));
//    ui->previewHtmlEdit->setProperty("dirty", QVariant(true));
//    ui->editTextEdit->setProperty("dirty", QVariant(true));
    ui->tabWidget->widget(1)->setProperty("dirty", QVariant(true));
    ui->tabWidget->widget(2)->setProperty("dirty", QVariant(true));
    ui->tabWidget->widget(3)->setProperty("dirty", QVariant(true));
}

void ImporterOptionsDialog::doSaveImport()
{
    QDialog::accept();
}

void ImporterOptionsDialog::doLoadFile()
{
    m_importer = 0;
    setDisableElements(true);

    ui->fileNameLineEdit->clear();
    m_content.clear();
    ui->previewTextEdit->clear();
    ui->contentTextEdit->clear();
    ui->previewHtmlEdit->clear();
    ui->tabWidget->setCurrentWidget(ui->contentTab);

    //ui->contentTextEdit->setProperty("dirty", QVariant(false));
//    ui->previewTextEdit->setProperty("dirty", QVariant(false));
//    ui->previewHtmlEdit->setProperty("dirty", QVariant(false));
//    ui->editTextEdit->setProperty("dirty", QVariant(false));
    ui->tabWidget->widget(1)->setProperty("dirty", QVariant(false));
    ui->tabWidget->widget(2)->setProperty("dirty", QVariant(false));
    ui->tabWidget->widget(3)->setProperty("dirty", QVariant(false));

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import..."), QDir::homePath(), ImporterManager::instance()->availableFormats().join(";;"));

    QFile file(fileName);
    QFileInfo fileInfo(fileName);

    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    m_content = QString::fromUtf8(file.readAll());
    const QString suffix = fileInfo.suffix().toLower();

    m_importer = ImporterManager::instance()->importer(suffix);

    if (m_importer && !m_content.isEmpty()) {
        setDisableElements(false);

        ui->fileNameLineEdit->setText(fileInfo.canonicalFilePath());
        ui->contentTextEdit->setText(m_content);

        ui->poemTitleLineEdit->setText(m_importer->options().poemStartPattern);

        ui->normalTextCheckBox->setChecked(m_importer->options().contentTypes & ImporterInterface::Options::NormalText);
        ui->classicCheckBox->setChecked(m_importer->options().contentTypes & ImporterInterface::Options::Poem);
        ui->whiteCheckBox->setChecked(m_importer->options().contentTypes & ImporterInterface::Options::WhitePoem);
    }
}
#include <QDebug>
void ImporterOptionsDialog::currentTabChanged()
{
//    qDebug() << "\n\n\n\n IS DIRTYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY: "
//             << ui->tabWidget->currentWidget()->property("dirty")
//             << ui->tabWidget->currentWidget()->property("type")
//             << ui->previewTextEdit->property("dirty")
//             << ui->previewTextEdit->property("type")
//             << index;
    if (ui->tabWidget->currentWidget()->property("dirty").toBool()) {
        ui->tabWidget->currentWidget()->setProperty("dirty", QVariant(false));

        if (ui->tabWidget->currentWidget() == ui->previewPlainTextTab) {
            qDebug() << "\n\n\n\n IS DIRTYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY: previewPlainTextTab";
            ui->previewTextEdit->setText(ImporterManager::instance()->convertTo(m_importer->importData(), ImporterManager::PlainText));
        }
        else if (ui->tabWidget->currentWidget() == ui->previewHtmlTab) {
            qDebug() << "\n\n\n\n IS DIRTYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY: previewHtmlTab";
            ui->previewHtmlEdit->setHtml(ImporterManager::instance()->convertTo(m_importer->importData(), ImporterManager::HtmlText));
        }
        else if (ui->tabWidget->currentWidget() == ui->editTab) {
            qDebug() << "\n\n\n\n IS DIRTYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY: editTab";
            ui->editTextEdit->setText(ImporterManager::instance()->convertTo(m_importer->importData(), ImporterManager::EditingText));
        }
//        qDebug() << "\n\n\n\n IS DIRTYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY: "
//                 << ui->tabWidget->currentWidget()->property("type");
    }
}

void ImporterOptionsDialog::setDisableElements(bool disable)
{
    ui->poemTitleLineEdit->setDisabled(disable);
    ui->normalTextCheckBox->setDisabled(disable);
    ui->classicCheckBox->setDisabled(disable);
    ui->whiteCheckBox->setDisabled(disable);
    ui->previewPushButton->setDisabled(disable);
}
