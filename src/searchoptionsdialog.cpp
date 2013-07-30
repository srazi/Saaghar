#include "searchoptionsdialog.h"
#include "ui_searchoptionsdialog.h"
#include "searchresultwidget.h"

SearchOptionsDialog::SearchOptionsDialog(QWidget* parent) :
    QDialog(parent, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint),
    ui(new Ui::SearchOptionsDialog)
{
    ui->setupUi(this);
    SearchResultWidget::nonPagedSearch = SearchResultWidget::nonPagedSearch ||
                                         SearchResultWidget::maxItemPerPage == 0;

    ui->nonPagedSearchCheckBox->setChecked(SearchResultWidget::nonPagedSearch);
    ui->maxResultSpinBox->setValue(SearchResultWidget::maxItemPerPage);
    ui->vowelSignsCheckBox->setChecked(SearchResultWidget::skipVowelSigns);
    ui->vowelLettersCheckBox->setChecked(SearchResultWidget::skipVowelLetters);

    connect(ui->searchTipsPushButton, SIGNAL(clicked()), parent, SLOT(showSearchTips()));
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

    SearchResultWidget::maxItemPerPage = ui->maxResultSpinBox->value();
    SearchResultWidget::nonPagedSearch = nonPaged;
    SearchResultWidget::skipVowelSigns = ui->vowelSignsCheckBox->isChecked();
    SearchResultWidget::skipVowelLetters = ui->vowelLettersCheckBox->isChecked();
    if (refreshRequired) {
        emit resultsRefreshRequired();
    }
    QDialog::accept();
}
