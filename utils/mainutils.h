/*
 * Copyright (C) 2023 YTX
 *
 * This file is part of YTX.
 *
 * YTX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * YTX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YTX. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <QtCore/qlibraryinfo.h>

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QProcess>

namespace utils {

inline QString ResourceFile()
{
    QString brc {};

#ifdef Q_OS_WIN
    brc = QCoreApplication::applicationDirPath() + "/resource";
    if (!QDir(brc).exists() && !QDir().mkpath(brc)) {
        qDebug() << "Failed to create directory:" << brc;
        return {};
    }
    brc += "/resource.brc";
#elif defined(Q_OS_MACOS)
    brc = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../Resources/resource.brc");
#endif

#if 0

    QString rcc { QLibraryInfo::path(QLibraryInfo::LibraryExecutablesPath) + "/rcc" };
#ifdef Q_OS_WIN
    rcc += ".exe";
#endif

    const QString qrc { QString(PROJECT_SOURCE_DIR) + "/resource/resource.qrc" };

    qDebug() << "rcc:" << rcc;
    qDebug() << "qrc:" << qrc;
    qDebug() << "brc:" << brc;

    const QStringList args { "-binary", qrc, "-o", brc };

    QProcess process {};
    process.start(rcc, args);
    process.waitForFinished();

    if (process.exitCode() != 0)
        qDebug() << "rcc failed:" << process.readAllStandardError();
#endif

    return brc;
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
