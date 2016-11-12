#include "searchoptionsdialog.h"
#include "ui_searchoptionsdialog.h"
#include "searchresultwidget.h"
#include "settingsmanager.h"

SearchOptionsDialog::SearchOptionsDialog(QWidget* parent) :
    QDialog(parent, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint),
    ui(new Ui::SearchOptionsDialog)
{
    ui->setupUi(this);
    SearchResultWidget::nonPagedSearch = SearchResultWidget::nonPagedSearch ||
                                         SearchResultWidget::maxItemPerPage == 0;

    ui->allRadioButton->setChecked(VARB("Search/Range/All"));
    ui->customRangeRadioButton->setChecked(VARB("Search/Range/Custom"));
    ui->openedTabsRadioButton->setChecked(VARB("Search/Range/OpenedTabs"));
    ui->titleRangeCheckBox->setChecked(VARB("Search/Range/Title"));

    ui->nonPagedSearchCheckBox->setChecked(SearchResultWidget::nonPagedSearch);
    ui->maxResultSpinBox->setValue(SearchResultWidget::maxItemPerPage);
    ui->vowelSignsCheckBox->setChecked(SearchResultWidget::skipVowelSigns);
    ui->vowelLettersCheckBox->setChecked(SearchResultWidget::skipVowelLetters);

    ui->selectionManager->setButtonBoxHidden(true);
    ui->selectionManager->parentsSelectChildren(true);
    ui->selectionManager->setSettingsPath(QLatin1String("Search/Range/CustomSelection"));

    ui->selectionManager->setEnabled(VARB("Search/Range/Custom"));
    connect(ui->customRangeRadioButton, SIGNAL(toggled(bool)), ui->selectionManager, SLOT(setEnabled(bool)));
    connect(ui->openedTabsRadioButton, SIGNAL(toggled(bool)), ui->titleRangeCheckBox, SLOT(setDisabled(bool)));
    connect(ui->clearPushButton, SIGNAL(clicked(bool)), ui->selectionManager, SLOT(clearSelection()));

    if (parent) {
        connect(ui->searchTipsPushButton, SIGNAL(clicked()), parent, SLOT(showSearchTips()));
    }
    else {
        ui->searchTipsPushButton->setDisabled(true);
    }
}

SearchOptionsDialog::~SearchOptionsDialog()
{
    delete ui;
}

void SearchOptionsDialog::accept()
{
    bool nonPaged = ui->nonPagedSearchCheckBox->isChecked() ||
                    ui->maxResultSpinBox->value() == 0;
    bool refreshRequired = ui->maxResultSpinBox->value() != SearchResultWidget::maxItemPerPage ||
                           nonPaged != SearchResultWidget::nonPagedSearch;

    VAR_DECL("Search/Range/All", ui->allRadioButton->isChecked());
    VAR_DECL("Search/Range/Custom", ui->customRangeRadioButton->isChecked());
    VAR_DECL("Search/Range/OpenedTabs", ui->openedTabsRadioButton->isChecked());
    VAR_DECL("Search/Range/Title", ui->titleRangeCheckBox->isChecked());

    // save selectionManager current selections
    ui->selectionManager->accept();

    SearchResultWidget::maxItemPerPage = ui->maxResultSpinBox->value();
    SearchResultWidget::nonPagedSearch = nonPaged;
    SearchResultWidget::skipVowelSigns = ui->vowelSignsCheckBox->isChecked();
    SearchResultWidget::skipVowelLetters = ui->vowelLettersCheckBox->isChecked();
    if (refreshRequired) {
        emit resultsRefreshRequired();
    }
    QDialog::accept();
}

#if QT_VERSION >= 0x050000
#include <QPainter>
#include <QPaintEvent>
#include "qtwin.h"
void SearchOptionsDialog::paintEvent(QPaintEvent* event)
{
    if (VARB("SaagharWindow/UseTransparecy") && QtWin::isCompositionEnabled()) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        p.fillRect(event->rect(), QColor(0, 0, 0, 0));
    }

    QDialog::paintEvent(event);
}
#endif
