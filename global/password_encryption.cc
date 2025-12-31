#include "password_encryption.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QProcess>
#include <QSettings>

QString PasswordEncryption::Encrypt(const QString& plaintext, const QByteArray& machine_key)
{
    if (plaintext.isEmpty() || machine_key.size() != KEY_SIZE) {
        return {};
    }

    QByteArray nonce(NONCE_SIZE, 0);
    if (RAND_bytes(reinterpret_cast<unsigned char*>(nonce.data()), nonce.size()) != 1) {
        qWarning() << "Failed to generate nonce";
        return {};
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning() << "Failed to create cipher context";
        return {};
    }

    if (EVP_EncryptInit_ex(ctx, EVP_chacha20_poly1305(), nullptr, reinterpret_cast<const unsigned char*>(machine_key.constData()),
            reinterpret_cast<const unsigned char*>(nonce.constData()))
        != 1) {
        qWarning() << "EncryptInit failed";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    const QByteArray plain_data { plaintext.toUtf8() };

    QByteArray ciphertext(plain_data.size() + TAG_SIZE, 0);
    int out_len = 0;

    if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(ciphertext.data()), &out_len, reinterpret_cast<const unsigned char*>(plain_data.constData()),
            plain_data.size())
        != 1) {
        qWarning() << "EncryptUpdate failed";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    int final_len = 0;
    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(ciphertext.data()) + out_len, &final_len) != 1) {
        qWarning() << "EncryptFinal failed";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    out_len += final_len;
    ciphertext.resize(out_len);

    QByteArray tag(TAG_SIZE, 0);
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, TAG_SIZE, tag.data()) != 1) {
        qWarning() << "Failed to get tag";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    EVP_CIPHER_CTX_free(ctx);

    QByteArray result {};
    result.append(nonce);
    result.append(ciphertext);
    result.append(tag);

    return QString::fromLatin1(result.toBase64());
}

QString PasswordEncryption::Decrypt(const QString& ciphertext, const QByteArray& machine_key)
{
    if (ciphertext.isEmpty() || machine_key.size() != KEY_SIZE) {
        return {};
    }

    const QByteArray all_data { QByteArray::fromBase64(ciphertext.toLatin1()) };
    if (all_data.size() < NONCE_SIZE + TAG_SIZE) { // nonce + tag
        qWarning() << "Invalid ciphertext (too short)";
        return {};
    }

    const QByteArray nonce { all_data.left(NONCE_SIZE) };
    const QByteArray tag { all_data.right(TAG_SIZE) };
    const QByteArray encrypted { all_data.mid(NONCE_SIZE, all_data.size() - NONCE_SIZE - TAG_SIZE) };

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning() << "Failed to create cipher context";
        return {};
    }

    if (EVP_DecryptInit_ex(ctx, EVP_chacha20_poly1305(), nullptr, reinterpret_cast<const unsigned char*>(machine_key.constData()),
            reinterpret_cast<const unsigned char*>(nonce.constData()))
        != 1) {
        qWarning() << "DecryptInit failed";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, TAG_SIZE, const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(tag.constData()))) != 1) {
        qWarning() << "Failed to set tag";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    QByteArray decrypted(encrypted.size(), 0);
    int out_len = 0;

    if (EVP_DecryptUpdate(
            ctx, reinterpret_cast<unsigned char*>(decrypted.data()), &out_len, reinterpret_cast<const unsigned char*>(encrypted.constData()), encrypted.size())
        != 1) {
        qWarning() << "DecryptUpdate failed";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    int final_len = 0;
    if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(decrypted.data()) + out_len, &final_len) != 1) {
        qWarning() << "DecryptFinal failed (tag verification failed or padding error)";
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    out_len += final_len;
    decrypted.resize(out_len);

    EVP_CIPHER_CTX_free(ctx);

    return QString::fromUtf8(decrypted);
}

QByteArray PasswordEncryption::GetMachineKey()
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

    return QCryptographicHash::hash(machine_id.toUtf8(), QCryptographicHash::Sha256);
}
