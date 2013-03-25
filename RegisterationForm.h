#ifndef REGISTERATIONFORM_H
#define REGISTERATIONFORM_H

#include <QDialog>

class QUrl;

namespace Ui
{
class RegisterationForm;
}

class RegisterationForm : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterationForm(QWidget* parent = 0);
    ~RegisterationForm();
    static bool showRegisterForm();

private:
    QHash<QString, QString> getInfo(const QString &email, const QString &pass, bool showProgress = true);
    bool sendEmail(const QString &email, const QString &key1, const QString &action, const QString &key2 = "0");
    bool _isRegisteredUser;
    void fillForm(const QHash<QString, QString> &dataHash);
    QString getHashedPassword(const QString &pass);
    QString getRemoteData(const QUrl &url, bool showProgress = true);
    Ui::RegisterationForm* ui;

private slots:
    void updateAskRegisterState();
    void forgotPass();
    void reSendEmail();
    void getInfo();
    void requiredChanged();
    void registeredButtonState();
    void tryToSubmit();
};

#endif // REGISTERATIONFORM_H
