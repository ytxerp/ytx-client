#include "password_encryption.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QProcess>
#include <QSettings>

QByteArray PasswordEncryption::DeriveKey(const QString& key) { return QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha256); }

QString PasswordEncryption::Encrypt(const QString& plaintext, const QString& key)
{
    if (plaintext.isEmpty()) {
        return {};
    }

    const QByteArray key_data { DeriveKey(key) };

    unsigned char iv[AES_BLOCK_SIZE];
    if (RAND_bytes(iv, AES_BLOCK_SIZE) != 1) {
        qWarning() << "Failed to generate IV";
        return {};
    }

    auto* ctx { EVP_CIPHER_CTX_new() };
    if (!ctx) {
        qWarning() << "Failed to create cipher context";
        return {};
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char*>(key_data.constData()), iv) != 1) {
        qWarning() << "Failed to initialize encryption";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    const QByteArray plain_data { plaintext.toUtf8() };
    const long long plain_len { plain_data.size() };

    QByteArray encrypted(plain_len + AES_BLOCK_SIZE, 0);
    int encrypted_len = 0;

    if (EVP_EncryptUpdate(
            ctx, reinterpret_cast<unsigned char*>(encrypted.data()), &encrypted_len, reinterpret_cast<const unsigned char*>(plain_data.constData()), plain_len)
        != 1) {
        qWarning() << "Failed to encrypt data";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    int finalLen = 0;
    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(encrypted.data()) + encrypted_len, &finalLen) != 1) {
        qWarning() << "Failed to finalize encryption";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    EVP_CIPHER_CTX_free(ctx);

    encrypted.resize(encrypted_len + finalLen);

    const QByteArray result { QByteArray(reinterpret_cast<const char*>(iv), AES_BLOCK_SIZE) + encrypted };
    return QString::fromLatin1(result.toBase64());
}

QString PasswordEncryption::Decrypt(const QString& ciphertext, const QString& key)
{
    if (ciphertext.isEmpty()) {
        return {};
    }

    const QByteArray encrypted_data { QByteArray::fromBase64(ciphertext.toLatin1()) };

    if (encrypted_data.size() < AES_BLOCK_SIZE) {
        qWarning() << "Invalid ciphertext length";
        return {};
    }

    unsigned char iv[AES_BLOCK_SIZE];
    memcpy(iv, encrypted_data.constData(), AES_BLOCK_SIZE);

    const QByteArray encrypted { encrypted_data.mid(AES_BLOCK_SIZE) };

    const QByteArray key_data { DeriveKey(key) };

    auto* ctx { EVP_CIPHER_CTX_new() };
    if (!ctx) {
        qWarning() << "Failed to create cipher context";
        return {};
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const unsigned char*>(key_data.constData()), iv) != 1) {
        qWarning() << "Failed to initialize decryption";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    QByteArray decrypted(encrypted.size(), 0);
    int decryptedLen = 0;

    if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(decrypted.data()), &decryptedLen, reinterpret_cast<const unsigned char*>(encrypted.constData()),
            encrypted.size())
        != 1) {
        qWarning() << "Failed to decrypt data";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    int finalLen = 0;
    if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(decrypted.data()) + decryptedLen, &finalLen) != 1) {
        qWarning() << "Failed to finalize decryption";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    EVP_CIPHER_CTX_free(ctx);

    decrypted.resize(decryptedLen + finalLen);
    return QString::fromUtf8(decrypted);
}

QString PasswordEncryption::GetMachineKey()
{
    QString machine_id {};

#ifdef Q_OS_WIN
    QSettings reg("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Cryptography", QSettings::NativeFormat);
    machine_id = reg.value("MachineGuid").toString();
#endif

#ifdef Q_OS_MAC
    QProcess process {};
    process.start("ioreg", QStringList() << "-rd1" << "-c" << "IOPlatformExpertDevice");
    process.waitForFinished();
    const QString output { process.readAllStandardOutput() };

    const long long start { output.indexOf("IOPlatformUUID") };
    if (start != -1) {
        const long long quote_start { output.indexOf("\"", start) };
        const long long quote_end { output.indexOf("\"", quote_start + 1) };
        machine_id = output.mid(quote_start + 1, quote_end - quote_start - 1);
    }
#endif

#ifdef Q_OS_LINUX
    QFile file("/etc/machine-id");
    if (file.open(QIODevice::ReadOnly)) {
        machine_id = QString::fromUtf8(file.readAll()).trimmed();
    }

    if (machine_id.isEmpty()) {
        QFile file2("/var/lib/dbus/machine-id");
        if (file2.open(QIODevice::ReadOnly)) {
            machine_id = QString::fromUtf8(file2.readAll()).trimmed();
        }
    }
#endif

    if (machine_id.isEmpty()) {
        qWarning() << "Failed to get machine ID, using default key";
        machine_id = "default-application-key-" + qAppName();
    }

    const QByteArray hash { QCryptographicHash::hash(machine_id.toUtf8(), QCryptographicHash::Sha256) };
    return QString::fromLatin1(hash.toHex());
}
