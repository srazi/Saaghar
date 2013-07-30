#ifndef REGISTERATIONFORM_H
#define REGISTERATIONFORM_H

#include <QDialog>

class QUrl;
class QTabWidget;

namespace Ui
{
class RegisterationForm;
}

class RegisterationForm : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterationForm(QWidget* parent = 0, QTabWidget* tabWidget = 0);
    ~RegisterationForm();
    static bool showRegisterForm();

public slots:
    void closeForm();

private:
    QHash<QString, QString> getInfo(const QString &email, const QString &pass, bool showProgress = true);
    bool sendEmail(const QString &email, const QString &key1, const QString &action, const QString &key2 = "0");
    void fillForm(const QHash<QString, QString> &dataHash);
    QString getHashedPassword(const QString &pass);
    QString getRemoteData(const QUrl &url, bool showProgress = true);

    bool m_isRegisteredUser;
    Ui::RegisterationForm* ui;
    QTabWidget* m_tabWidget;

private slots:
    void updateAskRegisterState();
    void forgotPass();
    void reSendEmail();
    void getInfo();
    void requiredChanged();
    void registeredButtonState();
    void tryToSubmit();

signals:
    void requestCloseRegisterationTab(int);
};

#endif // REGISTERATIONFORM_H
