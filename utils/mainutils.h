#ifndef MAINUTILS_H
#define MAINUTILS_H

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QProcess>
#include <QSslSocket>

namespace MainUtils {

inline QString ResourceFile()
{
    QString path {};

#ifdef Q_OS_WIN
    path = QCoreApplication::applicationDirPath() + "/resource";

    if (!QDir(path).exists() && !QDir().mkpath(path)) {
        qDebug() << "Failed to create directory:" << path;
        return {};
    }

    path += "/resource.brc";

#if 0
    QString command { "D:/Qt/6.9.3/llvm-mingw_64/bin/rcc.exe" };
    QStringList arguments {};
    arguments << "-binary"
              << "E:/ytx-client/resource/resource.qrc"
              << "-o" << path;

    QProcess process {};

    process.start(command, arguments);
    process.waitForFinished();
#endif

#elif defined(Q_OS_MACOS)
    path = QCoreApplication::applicationDirPath() + "/../Resources/resource.brc";

#if 0
    QString command { QDir::homePath() + "/Qt/6.9.3/macos/libexec/rcc" + " -binary " + QDir::homePath()
        + "/Documents/GitHub/ytx-client/resource/resource.qrc -o " + path };

    QProcess process {};
    process.start("zsh", QStringList() << "-c" << command);
    process.waitForFinished();
#endif

#endif

    return path;
}

inline void CheckOpenSSLRuntime()
{
    qDebug() << "SSL supported:" << QSslSocket::supportsSsl();

    if (!QSslSocket::supportsSsl()) {
        qCritical() << "❌ SSL NOT supported. OpenSSL not loaded!";
        return;
    }

    qDebug() << "SSL library build version:" << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << "SSL library runtime version:" << QSslSocket::sslLibraryVersionString();
    qDebug() << "Available SSL backends:" << QSslSocket::availableBackends();
    qDebug() << "Active SSL backend:" << QSslSocket::activeBackend();
}

inline void SetAppIcon(QApplication& app)
{
#ifdef Q_OS_WIN
    app.setWindowIcon(QIcon(":/logo/logo/logo.ico"));
#elif defined(Q_OS_MACOS)
    app.setWindowIcon(QIcon(":/logo/logo/logo.icns"));
#endif
}
} // namespace MainUtils

#endif // MAINUTILS_H
