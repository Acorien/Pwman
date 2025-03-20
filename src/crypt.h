#ifndef CRYPT_H
#define CRYPT_H

#include <QString>

class Crypt
{
public:
    static QByteArray encrypt(const QString &plainText, const QByteArray &key);
    static QString decrypt(const QByteArray &cipherText, const QByteArray &key);
    static QByteArray generateKey(const QString &password);
};

#endif
