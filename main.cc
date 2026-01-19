/*
 * Copyright (C) 2023 YtxErp
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

#include <QApplication>
#include <QDir>
#include <QResource>
#include <QStandardPaths>

#include "global/dailylogger.h"
#include "mainwindow/mainwindow.h"
#include "utils/mainutils.h"

int main(int argc, char* argv[])
{
    // Create the Qt application instance
    QApplication application(argc, argv);

    // Register resource file (contains icons, translations, themes, etc.)
    const QString resource_file { Utils::ResourceFile() };
    if (!QResource::registerResource(resource_file)) {
        qCritical() << "Failed to load application resources:" << resource_file;
        return EXIT_FAILURE;
    }

    // Use the Fusion style for a consistent cross-platform appearance
    application.setStyle("Fusion");

    // Disable native file dialogs to ensure consistent behavior across platforms
    application.setAttribute(Qt::AA_DontUseNativeDialogs);

    // Set application-level properties (requires resources)
    Utils::SetAppIcon(application);

    // Ensure the configuration directory exists
    const QString config_location { QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) };
    if (!QDir(config_location).exists() && !QDir().mkpath(config_location)) {
        qCritical() << "Failed to create config directory:" << config_location;
        return EXIT_FAILURE;
    }

    // Ensure the application data directory exists
    const QString data_location { QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) };
    if (!QDir(data_location).exists() && !QDir().mkpath(data_location)) {
        qCritical() << "Failed to create data directory:" << data_location;
        return EXIT_FAILURE;
    }

    // Install global logging system (DailyLogger)
    DailyLogger::Instance().Install();

    // Create the main window of the application
    MainWindow mainwindow {};

    // Show the main window
    mainwindow.show();

    // Start the Qt event loop
    const int result { application.exec() };

    // Cleanup: unregister resource file
    QResource::unregisterResource(resource_file);

    return result;
}
