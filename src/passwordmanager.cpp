#include "passwordmanager.h"
#include "crypt.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QFile>
#include <QTextStream>
#include <QRandomGenerator>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>

PasswordManager::PasswordManager(QWidget *parent, const QByteArray &key, bool cliMode)
    : QWidget(parent), encryptionKey(key)
{
    if(cliMode) {
        loadPasswords();
        return;
    }

    domainList = new QListWidget(this);
    domainEdit = new QLineEdit(this);
    domainEdit->setPlaceholderText("Enter domain name");

    createBtn = new QPushButton("Create", this);
    modifyBtn = new QPushButton("Modify", this);
    copyBtn = new QPushButton("Copy Password", this);
    viewBtn = new QPushButton("View Password", this);

    auto btnLayout = new QHBoxLayout;
    btnLayout->addWidget(createBtn);
    btnLayout->addWidget(modifyBtn);
    btnLayout->addWidget(copyBtn);
    btnLayout->addWidget(viewBtn);





    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(domainEdit);
    mainLayout->addWidget(domainList);
    mainLayout->addLayout(btnLayout);

    connect(createBtn, &QPushButton::clicked, this, &PasswordManager::createEntry);
    connect(copyBtn, &QPushButton::clicked, this, &PasswordManager::copyPassword);
    connect(viewBtn, &QPushButton::clicked, this, &PasswordManager::viewPassword);
    connect(modifyBtn, &QPushButton::clicked, this, &PasswordManager::openModifyDialog);

    loadPasswords();
}

QString PasswordManager::generateSecurePassword(int length)
{
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_-+=<>?";
    QString password;

    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.size());
        password.append(chars.at(index));
    }
    return password;
}

void PasswordManager::createEntry()
{
    QString domain = domainEdit->text().trimmed();
    if (domain.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a domain.");
        return;
    }

    if (passwords.contains(domain)) {
        QMessageBox::warning(this, "Error", "Domain already exists.");
        return;
    }

    // Ask for password length
    bool ok;
    int length = QInputDialog::getInt(this, "Password Length",
                                      "Enter password length (6-64):",
                                      16, // Default value
                                      6,  // Min value
                                      64, // Max value
                                      1,  // Step size
                                      &ok);
    if (!ok) {
        return; // User canceled input
    }

    // Generate password with chosen length
    QString password = generateSecurePassword(length);
    passwords[domain] = Crypt::encrypt(password, encryptionKey);
    domainList->addItem(domain);

    savePasswords();

    QMessageBox::information(this, "Password Created",
                             QString("New password for %1 (%2 chars):\n%3")
                                 .arg(domain)
                                 .arg(length)
                                 .arg(password));
}



void PasswordManager::copyPassword()
{
    if (!domainList->currentItem()) {
        QMessageBox::warning(this, "Error", "Select a domain to copy.");
        return;
    }

    QString domain = domainList->currentItem()->text();
    QString password = Crypt::decrypt(passwords[domain], encryptionKey);

    QApplication::clipboard()->setText(password);
    QMessageBox::information(this, "Copied", "Password copied to clipboard.");
}

void PasswordManager::viewPassword()
{
    if (!domainList->currentItem()) {
        QMessageBox::warning(this, "Error", "Select a domain to view the password.");
        return;
    }

    QString domain = domainList->currentItem()->text();
    QString password = Crypt::decrypt(passwords[domain], encryptionKey);

    QMessageBox::information(this, "View Password",
                             QString("Password for %1:\n%2").arg(domain, password));
}

void PasswordManager::generateNewPassword(int length)
{
    QString domain = domainList->currentItem()->text();
    QString newPassword = generateSecurePassword(length);
    passwords[domain] = Crypt::encrypt(newPassword, encryptionKey);
    savePasswords();
    modifyDialog->close();

    QMessageBox::information(this, "Password Generated",
                             QString("New password for %1 (%2 chars):\n%3")
                                 .arg(domain)
                                 .arg(length)
                                 .arg(newPassword));
}

void PasswordManager::openModifyDialog()
{
    if (!domainList->currentItem()) {
        QMessageBox::warning(this, "Error", "Select a domain to modify.");
        return;
    }

    QString domain = domainList->currentItem()->text();

    modifyDialog = new QWidget();
    modifyDialog->setWindowTitle(domain);
    modifyDialog->setAttribute(Qt::WA_DeleteOnClose);

    removeBtn = new QPushButton("Remove Domain");
    generateBtn = new QPushButton("Generate New Password");
    manualSetBtn = new QPushButton("Set Password Manually");
    manualPasswordEdit = new QLineEdit();
    manualPasswordEdit->setPlaceholderText("Enter new password");

    auto passwordLengthSpinBox = new QSpinBox();
    passwordLengthSpinBox->setMinimum(6);
    passwordLengthSpinBox->setMaximum(64);
    passwordLengthSpinBox->setValue(16);    // Default length
    passwordLengthSpinBox->setPrefix("Length: ");

    auto layout = new QVBoxLayout(modifyDialog);
    layout->addWidget(removeBtn);
    layout->addWidget(passwordLengthSpinBox);
    layout->addWidget(generateBtn);
    layout->addWidget(manualPasswordEdit);
    layout->addWidget(manualSetBtn);

    connect(removeBtn, &QPushButton::clicked, this, &PasswordManager::removeDomain);

    connect(generateBtn, &QPushButton::clicked, this, [this, passwordLengthSpinBox]() {
        generateNewPassword(passwordLengthSpinBox->value());
    });


    connect(manualSetBtn, &QPushButton::clicked, this, &PasswordManager::manuallySetPassword);

    modifyDialog->setLayout(layout);
    modifyDialog->resize(300, 150);
    modifyDialog->show();
}

void PasswordManager::removeDomain()
{
    QString domain = domainList->currentItem()->text();
    passwords.remove(domain);
    delete domainList->currentItem();
    savePasswords();
    modifyDialog->close();

    QMessageBox::information(this, "Removed", domain + " has been removed.");
}

void PasswordManager::generateNewPassword()
{
    QString domain = domainList->currentItem()->text();
    QString newPassword = generateSecurePassword();
    passwords[domain] = Crypt::encrypt(newPassword, encryptionKey);
    savePasswords();
    modifyDialog->close();

    QMessageBox::information(this, "Password Generated",
                             "New password for " + domain + ":\n" + newPassword);
}

void PasswordManager::manuallySetPassword()
{
    QString domain = domainList->currentItem()->text();
    QString newPassword = manualPasswordEdit->text().trimmed();

    if (newPassword.isEmpty()) {
        QMessageBox::warning(modifyDialog, "Error", "Password cannot be empty.");
        return;
    }

    passwords[domain] = Crypt::encrypt(newPassword, encryptionKey);
    savePasswords();
    modifyDialog->close();

    QMessageBox::information(this, "Password Updated",
                             "New password set manually for " + domain + ".");
}


void PasswordManager::loadPasswords()
{
    QFile file("cookiesfromthedarkside.dat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    passwords.clear();
    domainList->clear();

    while (!in.atEnd()) {
        QStringList parts = in.readLine().split('|');
        if (parts.size() == 2) {
            passwords[parts[0]] = parts[1].toUtf8();
            if(domainList)
                domainList->addItem(parts[0]);
        }
    }
}

void PasswordManager::savePasswords()
{
    QFile file("cookiesfromthedarkside.dat");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (auto it = passwords.begin(); it != passwords.end(); ++it) {
        out << it.key() << "|" << it.value() << "\n";
    }
}

void PasswordManager::cliMode()
{
    QTextStream cin(stdin), cout(stdout);
    cout << "CLI Mode:\n";
    QString choice;

    while (true) {
        cout << "\nOptions:\n"
             << "1. Add Domain\n"
             << "2. Get Password\n"
             << "3. Exit\n"
             << "Enter choice: " << Qt::flush;
        choice = cin.readLine().trimmed();

        if (choice == "1") {
            cout << "Enter domain: " << Qt::flush;
            QString domain = cin.readLine().trimmed();
            QString pwd = generateSecurePassword();
            passwords[domain] = Crypt::encrypt(pwd, encryptionKey);
            savePasswords();
            cout << "Password generated: " << pwd << "\n";
        }
        else if (choice == "2") {
            cout << "Enter domain: " << Qt::flush;
            QString domain = cin.readLine().trimmed();
            if (passwords.contains(domain)) {
                QString pwd = Crypt::decrypt(passwords[domain], encryptionKey);
                cout << "Password: " << pwd << "\n";
            } else {
                cout << "Domain not found.\n";
            }
        }
        else if (choice == "3") {
            break;
        }
        else {
            cout << "Invalid option.\n";
        }
    }
}
