#include "RegisterationForm.h"
#include "ui_registerationform.h"

#include "settings.h"

#include <QUrl>
#include <QDesktopServices>
#include <QDateTime>
#include <QCryptographicHash>
#include <QProgressDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTabWidget>

#include <QDebug>

const QString HOST = "http://pozh.org";
const QString REGISTER_PAGE = HOST + "/actions/register.php";
const QString EMAIL_PAGE = HOST + "/actions/email.php";
const QString INFO_PAGE = HOST + "/actions/get_info.php";
const QString RESET_PAGE = HOST + "/actions/reset.php";

RegisterationForm::RegisterationForm(QWidget* parent, QTabWidget* tabWidget)
    : QWidget(parent)
    , m_isRegisteredUser(false)
    , ui(new Ui::RegisterationForm)
    , m_tabWidget(tabWidget)
{
    ui->setupUi(this);

    ui->checkBoxIOS->hide();
    ui->checkBoxWinMobileOS->hide();
    ui->checkBoxMaemoOS->hide();
    ui->checkBoxSymbianOS->hide();
    ui->checkBoxAndroidOS->hide();
#ifdef Q_OS_WIN
    ui->checkBoxWinOS->setChecked(true);
#endif
#ifdef Q_OS_X11
    ui->checkBoxLinuxOS->setChecked(true);
#endif
#ifdef Q_OS_MAC
    ui->checkBoxMacOS->setChecked(true);
#endif

    ui->labelEmailVerified->hide();
    if (Settings::READ("RegisteredUser", false).toBool()) {
        ui->labelEmailVerified->show();
        QString email = Settings::READ("RegisteredEmail").toString();
        ui->lineEditEmailRegistered->setText(email);
        if (Settings::READ("EmailIsVerified", false).toBool()) {
            ui->labelEmailVerified->setText("<html><head/><body><p><span style=\" font-weight:600; color:#00ff00;\">Thanks, your email has been verified!</span></p></body></html>");
        }
        else {
            getInfo(email, "");
        }
        if (Settings::READ("EmailIsVerified", false).toBool()) {
            ui->groupBoxUserInfo->hide();
            ui->groupBoxUserType->hide();
            ui->groupBoxOSesType->hide();
            ui->groupBoxContribute->hide();
            ui->labelRequiredNote->hide();
            ui->pushButtonAskLater->hide();
            ui->pushButtonNeverAsk->hide();
            ui->pushButtonSubmit->hide();
        }
        adjustSize();
        //setFixedSize(size());
    }

    ui->pushButtonSubmit->setDisabled(true);
    connect(ui->lineEditLastName, SIGNAL(textChanged(QString)), this, SLOT(requiredChanged()));
    connect(ui->lineEditReTypePassword, SIGNAL(textChanged(QString)), this, SLOT(requiredChanged()));
    connect(ui->lineEditFirstName, SIGNAL(textChanged(QString)), this, SLOT(requiredChanged()));
    connect(ui->lineEditEmail, SIGNAL(textChanged(QString)), this, SLOT(requiredChanged()));
    connect(ui->lineEditPassword, SIGNAL(textChanged(QString)), this, SLOT(requiredChanged()));

    connect(ui->checkBoxWinOS, SIGNAL(stateChanged(int)), this, SLOT(requiredChanged()));
    connect(ui->checkBoxMacOS, SIGNAL(stateChanged(int)), this, SLOT(requiredChanged()));
    connect(ui->checkBoxLinuxOS, SIGNAL(stateChanged(int)), this, SLOT(requiredChanged()));
    connect(ui->checkBoxMeegoOS, SIGNAL(stateChanged(int)), this, SLOT(requiredChanged()));
    connect(ui->checkBoxIOS, SIGNAL(stateChanged(int)), this, SLOT(requiredChanged()));
    connect(ui->checkBoxWinMobileOS, SIGNAL(stateChanged(int)), this, SLOT(requiredChanged()));
    connect(ui->checkBoxMaemoOS, SIGNAL(stateChanged(int)), this, SLOT(requiredChanged()));
    connect(ui->checkBoxSymbianOS, SIGNAL(stateChanged(int)), this, SLOT(requiredChanged()));
    connect(ui->checkBoxAndroidOS, SIGNAL(stateChanged(int)), this, SLOT(requiredChanged()));
    connect(ui->lineEditOtherOS, SIGNAL(textChanged(QString)), this, SLOT(requiredChanged()));

    connect(ui->labelForgotPass, SIGNAL(linkActivated(QString)), this, SLOT(forgotPass()));
    connect(ui->pushButtonAskLater, SIGNAL(clicked()), this, SLOT(updateAskRegisterState()));
    connect(ui->pushButtonNeverAsk, SIGNAL(clicked()), this, SLOT(updateAskRegisterState()));
    connect(ui->pushButtonSubmit, SIGNAL(clicked()), this, SLOT(tryToSubmit()));
    connect(ui->pushButtonGetInfo, SIGNAL(clicked()), this, SLOT(getInfo()));
    connect(ui->pushButtonReSendEmail, SIGNAL(clicked()), this, SLOT(reSendEmail()));

    //registered button
    ui->pushButtonGetInfo->setDisabled(true);
    ui->pushButtonReSendEmail->setDisabled(true);
    ui->labelForgotPass->setDisabled(true);
    connect(ui->lineEditEmailRegistered, SIGNAL(textChanged(QString)), this, SLOT(registeredButtonState()));
    connect(ui->lineEditPasswordRegistered, SIGNAL(textChanged(QString)), this, SLOT(registeredButtonState()));

    if (m_tabWidget) {
        connect(this, SIGNAL(requestCloseRegisterationTab(int)), m_tabWidget, SIGNAL(tabCloseRequested(int)));
    }
}

RegisterationForm::~RegisterationForm()
{
    delete ui;
}

void RegisterationForm::requiredChanged()
{
    QString email = ui->lineEditEmail->text();
    bool submitDisabled = (ui->lineEditLastName->text().simplified().size() < 2)
//          || ui->lineEditReTypePassword->text().isEmpty()
                          || (ui->lineEditFirstName->text().simplified().size() < 2)
                          || email.isEmpty()
                          || (ui->lineEditPassword->text().size() < 4)
//          || ui->comboBoxOSes->currentText().isEmpty()
                          || (ui->lineEditPassword->text() != ui->lineEditReTypePassword->text())
                          || !email.contains(QRegExp("^[^@.\\s]\\S*@\\S*\\.\\S*[^@.\\s]$"));

    bool osIsSetted = ui->checkBoxWinOS->isChecked()
                      || ui->checkBoxMacOS->isChecked()
                      || ui->checkBoxLinuxOS->isChecked()
                      || ui->checkBoxMeegoOS->isChecked()
                      || ui->checkBoxIOS->isChecked()
                      || ui->checkBoxWinMobileOS->isChecked()
                      || ui->checkBoxMaemoOS->isChecked()
                      || ui->checkBoxSymbianOS->isChecked()
                      || ui->checkBoxAndroidOS->isChecked()
                      || !ui->lineEditOtherOS->text().isEmpty();

    submitDisabled |= !osIsSetted;

    if (ui->lineEditPassword->text() != ui->lineEditReTypePassword->text()) {
        ui->labelPassword->setText("<html><head/><body><p><span style=\" color:#ff0000;\">*Password:</span></p></body></html>");
        ui->labelReTypePassword->setText("<html><head/><body><p><span style=\" color:#ff0000;\">*ReType Pass.:</span></p></body></html>");
    }
    else {
        ui->labelPassword->setText("<html><head/><body><p><span style=\" color:#ff0000;\">*</span>Password:</p></body></html>");
        ui->labelReTypePassword->setText("<html><head/><body><p><span style=\" color:#ff0000;\">*</span>ReType Pass.:</p></body></html>");
    }

    ui->pushButtonSubmit->setDisabled(submitDisabled);
}

void RegisterationForm::registeredButtonState()
{
    QString r_email = ui->lineEditEmailRegistered->text();
    bool buttonsDisabled = r_email.isEmpty()
                           || ui->lineEditPasswordRegistered->text().isEmpty()
                           || !r_email.contains(QRegExp("^[^@.\\s]\\S*@\\S*\\.\\S*[^@.\\s]$"));
    ui->pushButtonGetInfo->setDisabled(buttonsDisabled);
    ui->pushButtonReSendEmail->setDisabled(buttonsDisabled);
    bool forgotLabelDisabled = r_email.isEmpty() || !r_email.contains(QRegExp("^[^@.\\s]\\S*@\\S*\\.\\S*[^@.\\s]$"));
    ui->labelForgotPass->setDisabled(forgotLabelDisabled);

    if (forgotLabelDisabled) {
        ui->labelForgotPass->setText("<html><head/><body><p><a href=\"#FORGOT_PASSWORD\"><span style=\" text-decoration: none; color:black;\"><i>Forgotten your password?</i></span></a></p></body></html>");
    }
    else {
        ui->labelForgotPass->setText("<html><head/><body><p><a href=\"#FORGOT_PASSWORD\"><span style=\" text-decoration: underline; color:#0000ff;\">Forgotten your password?</span></a></p></body></html>");
    }
}

void RegisterationForm::tryToSubmit()
{
    QUrl regUrl(REGISTER_PAGE);

    if (m_isRegisteredUser) {
        regUrl.addQueryItem("action", "renew");
        QString old_email = Settings::READ("RegisteredEmail").toString();
        regUrl.addQueryItem("old_email", QString::fromLocal8Bit(old_email.toLocal8Bit().toBase64().data()));
    }
    else {
        regUrl.addQueryItem("action", "register");
    }

    regUrl.addQueryItem("first_name", ui->lineEditFirstName->text());
    regUrl.addQueryItem("last_name", ui->lineEditLastName->text());
//  QByteArray pass = QString(ui->lineEditPassword->text()+"Saaghar_Pozh").toLocal8Bit();
//  pass = QCryptographicHash::hash(pass, QCryptographicHash::Sha1);
//  pass = QCryptographicHash::hash(pass, QCryptographicHash::Sha1)+pass;
//  pass = pass.toHex();
    QByteArray emailArrayB64 = ui->lineEditEmail->text().toLocal8Bit().toBase64();
    QString emailB64 = QString::fromLocal8Bit(emailArrayB64.data());
    qDebug() //<< "passfromLocal8Bit====" << QString::fromLocal8Bit(pass.data())<<"\npass="<<pass
            << "email==" << ui->lineEditEmail->text() << "\n" << emailB64 << "\n" <<
            ui->lineEditEmail->text().toLocal8Bit()
            << "\nemailHex=" << emailArrayB64 << "emailF=" << QByteArray::fromBase64(emailArrayB64);
    regUrl.addQueryItem("pass", getHashedPassword(ui->lineEditPassword->text()));
    QString gender = "n";
    if (ui->comboBoxGender->currentText() == tr("Female") || ui->comboBoxGender->currentText() == "Female") {
        gender = "f";
    }
    else if (ui->comboBoxGender->currentText() == tr("Male") || ui->comboBoxGender->currentText() == "Male") {
        gender = "m";
    }
    regUrl.addQueryItem("gender", gender);

    regUrl.addQueryItem("email", emailB64);
//  regUrl.addQueryItem("email_is_verified", "0");
    QString education = "";
    if (ui->comboBoxEducation->currentText() == tr("Student")) {
        education = "Student";
    }
    else if (ui->comboBoxEducation->currentText() == tr("BSc (student or graduated)")) {
        education = "BSc";
    }
    else if (ui->comboBoxEducation->currentText() == tr("MSc (student or graduated)")) {
        education = "MSc";
    }
    else if (ui->comboBoxEducation->currentText() == tr("PhD (student or graduated)")) {
        education = "PhD";
    }
    else if (ui->comboBoxEducation->currentText() == tr("Other")) {
        education = "Other";
    }

    regUrl.addQueryItem("education", education);

    QString field = "";
    if (ui->comboBoxFieldOfStudy->currentText() == tr("Mathematics")) {
        field = "Mathematics";
    }
    else if (ui->comboBoxFieldOfStudy->currentText() == tr("Computer")) {
        field = "Computer";
    }
    else if (ui->comboBoxFieldOfStudy->currentText() == tr("Literature")) {
        field = "Literature";
    }
    else if (ui->comboBoxFieldOfStudy->currentText() == tr("Art")) {
        field = "Art";
    }
    else if (ui->comboBoxFieldOfStudy->currentText() == tr("Human Sciences")) {
        field = "Human Sciences";
    }
    else if (ui->comboBoxFieldOfStudy->currentText() == tr("Engineering")) {
        field = "Engineering";
    }
    else if (ui->comboBoxFieldOfStudy->currentText() == tr("Basic Sciences")) {
        field = "Basic Sciences";
    }
    else if (ui->comboBoxFieldOfStudy->currentText() == tr("Other")) {
        field = "Other";
    }
    qDebug() << "fieldOO=" << field << "currentText=" << ui->comboBoxFieldOfStudy->currentText();
    regUrl.addQueryItem("field_study", field);

    QString date = QDateTime::currentDateTimeUtc().toString("yyyy-MM-ddThh:mm:ss");
    regUrl.addQueryItem("reg_date", date);
    QString key = QString::fromLocal8Bit(QString(date + ui->lineEditEmail->text()).toLocal8Bit().toHex().data());
    qDebug() << "KEY=" << key;
    regUrl.addQueryItem("key", key);

    regUrl.addQueryItem("normal_user", ui->checkBoxNormalApp->isChecked() ? "1" : "0");
    regUrl.addQueryItem("researcher_user", ui->checkBoxResearchApp->isChecked() ? "1" : "0");
    regUrl.addQueryItem("contributer", ui->groupBoxContribute->isChecked() ? "1" : "0");
    regUrl.addQueryItem("donator", ui->checkBoxDonation->isChecked() ? "1" : "0");
    regUrl.addQueryItem("dataset_creator", ui->checkBoxCreateNewDataSets->isChecked() ? "1" : "0");
    regUrl.addQueryItem("tester", ui->checkBoxTester->isChecked() ? "1" : "0");
    regUrl.addQueryItem("programmer", ui->checkBoxTester->isChecked() ? "1" : "0");
    regUrl.addQueryItem("usertype_other", ui->lineEditOtherContribute->text());

    regUrl.addQueryItem("windows", ui->checkBoxWinOS->isChecked() ? "1" : "0");
    regUrl.addQueryItem("mac", ui->checkBoxMacOS->isChecked() ? "1" : "0");
    regUrl.addQueryItem("linux", ui->checkBoxLinuxOS->isChecked() ? "1" : "0");
    regUrl.addQueryItem("meego", ui->checkBoxMeegoOS->isChecked() ? "1" : "0");
//  regUrl.addQueryItem("android", ui->checkBoxAndroidOS->isChecked() ? "1" : "0");
//  regUrl.addQueryItem("symbian", ui->checkBoxSymbianOS->isChecked() ? "1" : "0");
//  regUrl.addQueryItem("maemo", ui->checkBoxMaemoOS->isChecked() ? "1" : "0");
//  regUrl.addQueryItem("windows_mobile", ui->checkBoxWinMobileOS->isChecked() ? "1" : "0");
//  regUrl.addQueryItem("ios", ui->checkBoxIOS->isChecked() ? "1" : "0");
    regUrl.addQueryItem("os_other", ui->lineEditOtherOS->text());

    //QDesktopServices::openUrl(regUrl);
    qDebug() << "regUl=" << regUrl.toString();

    QString result = getRemoteData(regUrl);
    if (result.contains("Error")) {
        QString error = result.remove("Error!");
        if (error.isEmpty()) {
            error = tr("This email is present on database, if you want to modify your registeration information please use fields on the top of this form!");
        }
        else {
            error = tr("There was some errors!");
        }
        QMessageBox::information(this, tr("Error"), error);
        return;
    }
    else if (result.contains("Registeration Successful!")) {
        //write info to settings
        QString emailNote = "";
        if (!m_isRegisteredUser || Settings::READ("RegisteredEmail").toString() != ui->lineEditEmail->text()) {
            Settings::WRITE("EmailIsVerified", false);
            int i = 0;
            //send email
            if (sendEmail(emailB64, key, "activation")) {
                emailNote = tr("\nPlease check your email for verification link!");
            }
        }
        QMessageBox::information(this, tr("Registered!"), tr("Successfully registered!%1").arg(emailNote));
        Settings::WRITE("RegisteredUser", true);
        Settings::WRITE("RegisteredEmail", ui->lineEditEmail->text());
        closeForm();
    }
}

void RegisterationForm::getInfo()
{
    QString r_email = ui->lineEditEmailRegistered->text();
    QString r_pass = ui->lineEditPasswordRegistered->text();
////////////////////////////////////////////////////////////////////
    //if (r_email.isEmpty() || r_pass.isEmpty()) return;
    QHash<QString, QString> infoHash = getInfo(r_email, r_pass);
    if (!infoHash.isEmpty()) {
        fillForm(infoHash);
        ui->groupBoxUserInfo->show();
        ui->groupBoxUserType->show();
        ui->groupBoxOSesType->show();
        ui->groupBoxContribute->show();
        ui->labelRequiredNote->show();
        ui->pushButtonSubmit->show();
        ui->pushButtonSubmit->setText(tr("Re-Submit"));
        adjustSize();
    }
//  setFixedSize(size());
}

QString RegisterationForm::getRemoteData(const QUrl &url, bool showProgress)
{
    //QMessageBox::information(0, "URL:",tr("URL:\n%1").arg(url.toString()));
    //QDesktopServices::openUrl(url);
    QEventLoop loop;

    QNetworkRequest requestData(url);
    QNetworkAccessManager* netManager = new QNetworkAccessManager();
    QNetworkReply* reply = netManager->get(requestData);

    QProgressDialog dataTransferProgress(tr("Get/Send Data..."),  tr("Cancel"), 0, 0, this);
    dataTransferProgress.setMinimumDuration(1000);

    dataTransferProgress.setWindowModality(Qt::WindowModal);
    dataTransferProgress.setFixedSize(dataTransferProgress.size());
    if (showProgress) {
        dataTransferProgress.show();
    }
    else {
        dataTransferProgress.hide();
    }

    connect(&dataTransferProgress, SIGNAL(canceled()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), &dataTransferProgress, SLOT(hide()));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (dataTransferProgress.wasCanceled()) {
        dataTransferProgress.hide();
        loop.quit();
        return "Error!CanceledByUser";
    }

    if (reply->error()) {
        QMessageBox criticalError(QMessageBox::Critical, tr("Error"), tr("There is an error when transfering data to/from remote data base...\nError: %1").arg(reply->errorString()), QMessageBox::Ok, this, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);
        criticalError.exec();
        return "Error!NetworkError";
    }
    QString data = QString::fromUtf8(reply->readAll());
    //QMessageBox::information(0, "remote data1",tr("remote-data1=%1").arg(data));
    qDebug() << "data1=" << data;

    if (data.contains("<body>")) {
        data.remove(0, data.indexOf("<body>") + 6);
    }
    data.remove("</body>");
    data.remove("</html>");
    //QMessageBox::information(0, "remote data2",tr("remote-data2=%1").arg(data));
    qDebug() << "data2=" << data;
    data.remove(QRegExp("[\\n\\r]+"));
    //QMessageBox::information(0, "remote data3",tr("remote-data3=%1").arg(data));
    qDebug() << "data3=" << data;
    return data;
}

void RegisterationForm::updateAskRegisterState()
{
    if (sender() == ui->pushButtonAskLater) {
        QStringList tmpList = Settings::READ("RegisterationState").toStringList();
        int numOfRun = 0;
        uint lastTime = 0;
        uint firstTime = 0;
        if (tmpList.size() == 4 && tmpList.at(0) == "AskLater") {
            firstTime = tmpList.at(1).toUInt();
            lastTime = tmpList.at(2).toUInt();
            numOfRun = tmpList.at(3).toInt();
        }
        uint currentTime = QDateTime::currentDateTimeUtc().toTime_t();
        if (lastTime >= currentTime) {
            currentTime = lastTime + 1200;
        }
        else if (lastTime == 0) {
            lastTime = currentTime;
        }
        if (firstTime == 0) {
            firstTime = currentTime;
        }

        Settings::WRITE("RegisterationState",
                        QStringList()
                        << "AskLater"
                        << QString::number(firstTime)
                        << QString::number(currentTime)
                        << QString::number(numOfRun));
        closeForm();
    }
    else if (sender() == ui->pushButtonNeverAsk) {
        if (QMessageBox::information(this, tr("Warning"),
                                     tr("Do you want to cancel registeration process?\n"
                                        "The registeration\'s dialog is accessible from \"Help->Registeration\" menu."),
                                     QMessageBox::Ok | QMessageBox::Cancel)
                == QMessageBox::Ok) {
            Settings::WRITE("RegisterationState", QStringList() << "NeverASk");
            closeForm();
        }
    }
}

//static
bool RegisterationForm::showRegisterForm()
{
    if (Settings::READ("RegisteredUser", false).toBool()) {
        return false;
    }

    const int NUM_LAUNCH = 500;
    const uint NUM_DAYS_SEC = 7 * (24 * 60 * 60);
    QStringList tmpList = Settings::READ("RegisterationState").toStringList();
    if (tmpList.isEmpty()) {
        return true;
    }
    if (tmpList.at(0) == "NeverASk") {
        return false;
    }
    if (tmpList.size() != 4 || tmpList.at(0) != "AskLater") {
        return true;
    }
    uint firstTime = tmpList.at(1).toUInt();
    uint lastTime = tmpList.at(2).toUInt();
    int numOfRun = tmpList.at(3).toInt() + 1;
    uint currentTime = QDateTime::currentDateTimeUtc().toTime_t();
    if (currentTime - firstTime >= NUM_DAYS_SEC || numOfRun >= NUM_LAUNCH) {
        Settings::WRITE("RegisterationState", QStringList());
        return true;
    }
    else {
        Settings::WRITE("RegisterationState",
                        QStringList()
                        << "AskLater"
                        << QString::number(firstTime)
                        << QString::number(currentTime)
                        << QString::number(numOfRun));
        return false;
    }
}

void RegisterationForm::closeForm()
{
    if (m_tabWidget) {
        for (int i = 0; i < m_tabWidget->count(); ++i) {
            if (m_tabWidget->widget(i) && m_tabWidget->widget(i)->objectName() == "WidgetTab-Registeration") {
                emit requestCloseRegisterationTab(i);
                return;
            }
        }
    }
    else {
        close();
    }
}

QString RegisterationForm::getHashedPassword(const QString &pass)
{
    qDebug() << "pass1=" << pass;
    QByteArray pass_array = QString(pass + "~Saaghar!Pozh").toLocal8Bit();
    pass_array = QCryptographicHash::hash(pass_array, QCryptographicHash::Sha1);
    qDebug() << "pass2=" << pass_array.toHex() << "--\nSubStr=" << pass_array.toHex().left(10);
    //qDebug() << "pass_length="<<pass_array.toHex();
    qDebug() << "pass3=" << QCryptographicHash::hash(pass_array, QCryptographicHash::Sha1).toHex();
    pass_array = QCryptographicHash::hash(pass_array, QCryptographicHash::Sha1) + pass_array.toHex().left(10);
    qDebug() << "pass4=" << pass_array.toHex();
    pass_array = pass_array.toHex();
    qDebug() << "pass_length=" << pass_array.size() << "pass=" << QString::fromLocal8Bit(pass_array.data());
    return QString::fromLocal8Bit(pass_array.data());
}

void RegisterationForm::fillForm(const QHash<QString, QString> &dataHash)
{
//  dataHash.value("pass");
//  dataHash.value("user_id");
    ui->lineEditFirstName->setText(dataHash.value("first_name"));
    ui->lineEditLastName->setText(dataHash.value("last_name"));
    QString gender = dataHash.value("gender", "n");
    if (gender != "n") {
        if (gender == "m") {
            gender = tr("Male");
        }
        else {
            gender = tr("Female");
        }

        ui->comboBoxGender->setCurrentIndex(ui->comboBoxGender->findText(gender));
    }
    QString email = QString::fromLocal8Bit(QByteArray::fromBase64(dataHash.value("email").toLocal8Bit()));
    ui->lineEditEmail->setText(email);
    dataHash.value("email_is_verified", "0");
    QString education = dataHash.value("education");
    QString field = dataHash.value("field_study");

    if (education == "Student") {
        education = tr("Student");
    }
    else if (education == "BSc") {
        education = tr("BSc (student or graduated)");
    }
    else if (education == "MSc") {
        education = tr("MSc (student or graduated)");
    }
    else if (education == "PhD") {
        education = tr("PhD (student or graduated)");
    }
    else if (education == "Other") {
        education = tr("Other");
    }

    ui->comboBoxEducation->setCurrentIndex(ui->comboBoxEducation->findText(education));
    qDebug() << "field111=" << field << "index=" << ui->comboBoxFieldOfStudy->findText(field);
    if (field == "Mathematics") {
        field = tr("Mathematics");
    }
    else if (field == "Computer") {
        field = tr("Computer");
    }
    else if (field == "Literature") {
        field = tr("Literature");
    }
    else if (field == "Art") {
        field = tr("Art");
    }
    else if (field == "Human Sciences") {
        field = tr("Human Sciences");
    }
    else if (field == "Engineering") {
        field = tr("Engineering");
    }
    else if (field == "Basic Sciences") {
        field = tr("Basic Sciences");
    }
    else if (field == "Other") {
        field = tr("Other");
    }
    qDebug() << "field222=" << field << "index=" << ui->comboBoxFieldOfStudy->findText(field);
    ui->comboBoxFieldOfStudy->setCurrentIndex(ui->comboBoxFieldOfStudy->findText(field));

//  dataHash.value("reg_date");
//  dataHash.value("key");
    ui->checkBoxNormalApp->setChecked(dataHash.value("normal_user", "0") != "0");
    ui->checkBoxResearchApp->setChecked(dataHash.value("researcher_user", "0") != "0");
    ui->groupBoxContribute->setChecked(dataHash.value("contributer", "0") != "0");
    ui->checkBoxDonation->setChecked(dataHash.value("donator", "0") != "0");
    ui->checkBoxCreateNewDataSets->setChecked(dataHash.value("dataset_creator", "0") != "0");
    ui->checkBoxTester->setChecked(dataHash.value("tester", "0") != "0");
    ui->checkBoxProgrammer->setChecked(dataHash.value("programmer", "0") != "0");
    ui->lineEditOtherContribute->setText(dataHash.value("usertype_other"));
    ui->checkBoxWinOS->setChecked(dataHash.value("windows", "0") != "0");
    ui->checkBoxMacOS->setChecked(dataHash.value("mac", "0") != "0");
    ui->checkBoxLinuxOS->setChecked(dataHash.value("linux", "0") != "0");
    ui->checkBoxMeegoOS->setChecked(dataHash.value("meego", "0") != "0");
//  ui->checkBoxAndroidOS->setChecked( dataHash.value("android", "0")!="0" );
//  ui->checkBoxSymbianOS->setChecked( dataHash.value("symbian", "0")!="0" );
//  ui->checkBoxMaemoOS->setChecked( dataHash.value("maemo", "0")!="0" );
//  ui->checkBoxWinMobileOS->setChecked( dataHash.value("windows_mobile", "0")!="0" );
//  ui->checkBoxIOS->setChecked( dataHash.value("ios", "0")!="0" );
    ui->lineEditOtherOS->setText(dataHash.value("os_other"));
}

bool RegisterationForm::sendEmail(const QString &email, const QString &key1, const QString &action, const QString &key2)
{
    QUrl sendEmailUrl(EMAIL_PAGE);
    sendEmailUrl.addQueryItem("a", action);
    sendEmailUrl.addQueryItem("e", email);
    sendEmailUrl.addQueryItem("key1", key1);
    sendEmailUrl.addQueryItem("key2", key2);
//QDesktopServices::openUrl(sendEmailUrl);
    QString rawInfo = getRemoteData(sendEmailUrl);
    qDebug() << "sendActivationEmail=" << rawInfo;
    if (rawInfo != "MessageSuccessfullySent!") {
        return false;
    }
    else {
        return true;
    }
}

void RegisterationForm::reSendEmail()
{
//  QByteArray emailArrayB64 = ui->lineEditEmailRegistered->text().toLocal8Bit().toBase64();
//  qDebug() << "r_emailArrayHex====" << QString::fromLocal8Bit(emailArrayB64.data());
//  getInfoUrl.addQueryItem("pass", getHashedPassword(r_pass));
//  getInfoUrl.addQueryItem("email", QString::fromLocal8Bit(emailArrayB64.data()));

    QHash<QString, QString> infoHash = getInfo(ui->lineEditEmailRegistered->text(), ui->lineEditPasswordRegistered->text());
    if (sendEmail(infoHash.value("email"), infoHash.value("key"), "activation")) {
        QMessageBox::information(this, tr("Verification email was sent!"), tr("Please check your email for verification link!"));
    }
}

QHash<QString, QString> RegisterationForm::getInfo(const QString &email, const QString &pass, bool showProgress)
{
    QHash<QString, QString> registeredUserData;
    if (email.isEmpty()) {
        return registeredUserData;
    }

    QUrl getInfoUrl(INFO_PAGE);


    QByteArray emailArrayB64 = email.toLocal8Bit().toBase64();
    qDebug() << "r_emailArrayHex====" << QString::fromLocal8Bit(emailArrayB64.data());
    if (!pass.isEmpty()) { //getting all data
        getInfoUrl.addQueryItem("pass", getHashedPassword(pass));
    }
    else { //getting just 'email_is_verified'
        getInfoUrl.addQueryItem("pass", "1");
    }

    getInfoUrl.addQueryItem("email", QString::fromLocal8Bit(emailArrayB64.data()));

    //QDesktopServices::openUrl(getInfoUrl);
    qDebug() << "getInfoUrl=" << getInfoUrl.toString();
    QString rawInfo = getRemoteData(getInfoUrl, showProgress);

    if (!pass.isEmpty()) {
        if (rawInfo.contains("Error") || !rawInfo.contains("first_name")) {
            QString error = rawInfo.remove("Error!");
            if (error.isEmpty()) {
                error = tr("Username or password is mistake or this email is not registered, yet.");
            }
            else {
                error = tr("There was some errors!");
            }
            QMessageBox::information(this, tr("Error"), error);
            return registeredUserData;
        }
        m_isRegisteredUser = true;
    }
    QStringList rawList = rawInfo.split("<br />", QString::SkipEmptyParts);
//  QHash<QString, QString> registeredUserData;
    for (int i = 0; i < rawList.size(); ++i) {
        QStringList tmp = rawList.at(i).split("=");
        registeredUserData.insert(tmp.at(0), tmp.size() > 1 ? tmp.at(1) : "");
    }

    if (registeredUserData.value("email_is_verified", "0") == "1") {
        Settings::WRITE("EmailIsVerified", true);
        ui->labelEmailVerified->setText("<html><head/><body><p><span style=\" font-weight:600; color:#00ff00;\">Thanks, your email has been verified!</span></p></body></html>");
    }
    return registeredUserData;
}

void RegisterationForm::forgotPass()
{
    QString newPass = QString::number(qrand()) + QString::number(qrand());
    //QString::fromLocal8Bit(QString::number(qrand()).toLocal8Bit().toHex().data());

//  QMessageBox::information(this, tr("New Password!"),
//  tr("The reset link was sent to \"%1\" for finish reset process you must to click on reset link!"
//     "<br>New password: <b>%2</b> you need this after resetting!").arg(ui->lineEditEmailRegistered->text()).arg(newPass));

//  return;
    QString email = ui->lineEditEmailRegistered->text();
    QString emailB64 = QString::fromLocal8Bit(email.toLocal8Bit().toBase64().data());
    QUrl resetUrl(RESET_PAGE);
    QString key = QString::number(qrand()) + QString::number(qrand()) + QString::number(qrand()) + QString::number(qrand());
    key = QCryptographicHash::hash(key.toLocal8Bit(), QCryptographicHash::Sha1).toHex();
    //QString::fromLocal8Bit(key.toLocal8Bit().toBase64().toHex().data());
    resetUrl.addQueryItem("a", "makekey");
    resetUrl.addQueryItem("e", emailB64);
    resetUrl.addQueryItem("k", key);

    QString rawInfo = getRemoteData(resetUrl);
    qDebug() << "resetKey=" << rawInfo << "\n" << resetUrl;

    if (!rawInfo.contains("KeyCreated!")) {
        if (rawInfo.contains("Error!NotRegistered")) {
            if (email == Settings::READ("RegisteredEmail").toString()) {
                Settings::WRITE("EmailIsVerified", false);
                Settings::READ("RegisteredUser", false);
                Settings::READ("RegisteredEmail", "");
                QMessageBox::information(this, tr("Error"), tr("You are not registered!"));
                ui->groupBoxUserInfo->show();
                ui->groupBoxUserType->show();
                ui->groupBoxOSesType->show();
                ui->groupBoxContribute->show();
                ui->labelRequiredNote->show();
                ui->pushButtonSubmit->show();
                ui->pushButtonSubmit->setText(tr("Submit"));
                adjustSize();
                return;
            }
        }

        QMessageBox::information(this, tr("Error"), tr("There was an error!"));
        return;
    }

    //QHash<QString, QString> infoHash = getInfo(email, "");
    qDebug() << "newPass=" << newPass;
    if (QMessageBox::Ok ==
            QMessageBox::information(this, tr("Password Reset!"),
                                     tr("By clicking on the \"Ok\" button the password's reset link will be send to this email address: %1\nAre you sure?")
                                     .arg(ui->lineEditEmailRegistered->text()), QMessageBox::Ok | QMessageBox::Cancel)) {
        //regUrl.addQueryItem("pass", getHashedPassword(newPass));nP=%2;
        if (sendEmail(emailB64, key/*infoHash.value("key")*/, "reset",  getHashedPassword(newPass))) {
            QMessageBox::information(this, tr("New Password!"),
                                     tr("The reset link was sent to \"%1\" for finish reset process you must to click on reset link!"
                                        "<br>New password: <b>%2</b> you need this after resetting!").arg(ui->lineEditEmailRegistered->text()).arg(newPass));
        }
        else {
            QMessageBox::information(this, tr("Error"), tr("There was an error, the email was not sent!"));
        }
    }
}
