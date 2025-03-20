#include "crypt.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <QCryptographicHash>

QByteArray Crypt::generateKey(const QString &password)
{
    return QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
}

QByteArray Crypt::encrypt(const QString &plainText, const QByteArray &key)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    unsigned char iv[AES_BLOCK_SIZE];
    if (!RAND_bytes(iv, AES_BLOCK_SIZE)) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();  // Failed to generate IV
    }

    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                            reinterpret_cast<const unsigned char*>(key.data()), iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();  // Encryption initialization failed
    }

    QByteArray cipherText;
    cipherText.resize(plainText.size() + AES_BLOCK_SIZE);

    int len = 0;
    int cipherLen = 0;

    if (!EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(cipherText.data()), &len,
                           reinterpret_cast<const unsigned char*>(plainText.toUtf8().data()), plainText.size())) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();  // Encryption failed
    }

    cipherLen = len;

    if (!EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(cipherText.data()) + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();  // Final encryption step failed
    }

    cipherLen += len;
    cipherText.resize(cipherLen);

    EVP_CIPHER_CTX_free(ctx);

    QByteArray result = QByteArray(reinterpret_cast<char*>(iv), AES_BLOCK_SIZE) + cipherText;
    return result.toBase64();
}

QString Crypt::decrypt(const QByteArray &cipherText, const QByteArray &key)
{
    QByteArray data = QByteArray::fromBase64(cipherText);

    if (data.size() <= AES_BLOCK_SIZE) {
        return QString();  // Data too short
    }

    unsigned char iv[AES_BLOCK_SIZE];
    memcpy(iv, data.data(), AES_BLOCK_SIZE);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                            reinterpret_cast<const unsigned char*>(key.data()), iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();  // Decryption initialization failed
    }

    QByteArray plainText;
    plainText.resize(data.size());

    int len = 0;
    int plainLen = 0;

    if (!EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(plainText.data()), &len,
                           reinterpret_cast<const unsigned char*>(data.data()) + AES_BLOCK_SIZE,
                           data.size() - AES_BLOCK_SIZE)) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();  // Decryption failed
    }

    plainLen = len;

    if (!EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(plainText.data()) + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return QString();  // Final decryption step failed
    }

    plainLen += len;
    plainText.resize(plainLen);

    EVP_CIPHER_CTX_free(ctx);

    return QString::fromUtf8(plainText);
}
