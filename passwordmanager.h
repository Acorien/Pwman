#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include <QWidget>
#include <QMap>

class QListWidget;
class QPushButton;
class QLineEdit;

class PasswordManager : public QWidget
{
    Q_OBJECT

public:
    PasswordManager(QWidget *parent, const QByteArray &key, bool cliMode = false);

    void cliMode();

private slots:
    void createEntry();
    void copyPassword();
    void viewPassword();
    void generateNewPassword(int length);

    void openModifyDialog();
    void removeDomain();
    void generateNewPassword();
    void manuallySetPassword();

private:
    QString generateSecurePassword(int length = 16);
    void loadPasswords();
    void savePasswords();

    QListWidget *domainList;
    QPushButton *createBtn, *modifyBtn, *copyBtn, *viewBtn;
    QLineEdit *domainEdit;


    QWidget *modifyDialog;
    QPushButton *removeBtn, *generateBtn, *manualSetBtn;
    QLineEdit *manualPasswordEdit;

    QMap<QString, QByteArray> passwords;
    QByteArray encryptionKey;
};

#endif // PASSWORDMANAGER_H
