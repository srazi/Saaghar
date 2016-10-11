#include "importeroptionsdialog.h"
#include "ui_importeroptionsdialog.h"
#include "importer_interface.h"
#include "importermanager.h"
#include "qtwin.h"
#include "settingsmanager.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>

ImporterOptionsDialog::ImporterOptionsDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::ImporterOptionsDialog),
    m_importer(0),
    m_contentViewInDirtyState(true)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    ui->setupUi(this);

    // ui->contentTextEdit->setReadOnly(true);
    ui->previewTextEdit->setReadOnly(true);
    ui->previewHtmlEdit->setReadOnly(true);

    setDisableElements(true);
    //ui->importPushButton->setDisabled(true);
    ui->tabWidget->removeTab(3);

    connect(ui->contentTextEdit, SIGNAL(textChanged()), this, SLOT(contentChanghed()));
    connect(ui->browsePushButton, SIGNAL(clicked(bool)), this, SLOT(doLoadFile()));
    connect(ui->previewPushButton, SIGNAL(clicked(bool)), this, SLOT(doImportPreview()));
    connect(ui->importPushButton, SIGNAL(clicked(bool)), this, SLOT(doSaveImport()));
    connect(ui->cancelPushButton, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged()));

    QtWin::easyBlurUnBlur(this, VARB("SaagharWindow/UseTransparecy"));
}

ImporterOptionsDialog::~ImporterOptionsDialog()
{
    delete ui;
}

void ImporterOptionsDialog::doImportPreview()
{
    m_content = ui->contentTextEdit->toPlainText();

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

    ui->importPushButton->setDisabled(false);

    ui->tabWidget->widget(1)->setProperty("dirty", QVariant(true));
    ui->tabWidget->widget(2)->setProperty("dirty", QVariant(true));
//    ui->tabWidget->widget(3)->setProperty("dirty", QVariant(true));
}

void ImporterOptionsDialog::doSaveImport()
{
    m_content = ui->contentTextEdit->toPlainText();

    if (m_content.isEmpty() || !m_importer) {
        return;
    }

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

    ImporterManager::instance()->storeAsDataset(m_importer->importData());

    QDialog::accept();
}

void ImporterOptionsDialog::contentChanghed()
{
    if (!m_contentViewInDirtyState) {
        return;
    }

    const QString content = ui->contentTextEdit->toPlainText();

    if (content.isEmpty() || content == m_content) {
        return;
    }

    init(content);
}

void ImporterOptionsDialog::reset()
{
    m_importer = 0;
    setDisableElements(true);

    ui->fileNameLineEdit->clear();
    m_content.clear();
    ui->previewTextEdit->clear();
    ui->contentTextEdit->clear();
    ui->previewHtmlEdit->clear();
    ui->tabWidget->setCurrentWidget(ui->contentTab);

    ui->tabWidget->widget(1)->setProperty("dirty", QVariant(false));
    ui->tabWidget->widget(2)->setProperty("dirty", QVariant(false));
//    ui->tabWidget->widget(3)->setProperty("dirty", QVariant(false));

    m_contentViewInDirtyState = true;
}

void ImporterOptionsDialog::init(const QString &content, const QString &type)
{
    m_content = content;

    m_importer = ImporterManager::instance()->importer(type);

    if (m_importer && !m_content.isEmpty()) {
        setDisableElements(false);

        ui->contentTextEdit->setText(m_content);
        ui->poemTitleLineEdit->setText(m_importer->options().poemStartPattern);

        ui->normalTextCheckBox->setChecked(m_importer->options().contentTypes & ImporterInterface::Options::NormalText);
        ui->classicCheckBox->setChecked(m_importer->options().contentTypes & ImporterInterface::Options::Poem);
        ui->whiteCheckBox->setChecked(m_importer->options().contentTypes & ImporterInterface::Options::WhitePoem);
    }

    m_contentViewInDirtyState = false;
}

void ImporterOptionsDialog::doLoadFile()
{
    reset();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Select File To Import..."), QDir::homePath(), ImporterManager::instance()->availableFormats().join(";;"));

    QFile file(fileName);
    QFileInfo fileInfo(fileName);

    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    init(QString::fromUtf8(file.readAll()), fileInfo.suffix().toLower());

    ui->fileNameLineEdit->setText(fileInfo.canonicalFilePath());
}

void ImporterOptionsDialog::currentTabChanged()
{
    if (ui->tabWidget->currentWidget()->property("dirty").toBool()) {
        ui->tabWidget->currentWidget()->setProperty("dirty", QVariant(false));

        if (ui->tabWidget->currentWidget() == ui->previewPlainTextTab) {
            ui->previewTextEdit->setText(ImporterManager::instance()->convertTo(m_importer->importData(), ImporterManager::PlainText));
        }
        else if (ui->tabWidget->currentWidget() == ui->previewHtmlTab) {
            ui->previewHtmlEdit->setHtml(ImporterManager::instance()->convertTo(m_importer->importData(), ImporterManager::HtmlText));
        }
//        else if (ui->tabWidget->currentWidget() == ui->editTab) {
//            ui->editTextEdit->setText(ImporterManager::instance()->convertTo(m_importer->importData(), ImporterManager::EditingText));
//        }
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

#if QT_VERSION >= 0x050000
#include <QPainter>
#include <QPaintEvent>
void ImporterOptionsDialog::paintEvent(QPaintEvent* event)
{
    if (VARB("SaagharWindow/UseTransparecy") && QtWin::isCompositionEnabled()) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        p.fillRect(event->rect(), QColor(0, 0, 0, 0));
    }

    QDialog::paintEvent(event);
}
#endif
