#include "dailylogger.h"

#include <QDir>
#include <QStandardPaths>

DailyLogger& DailyLogger::Instance()
{
    static DailyLogger instance {};
    return instance;
}

DailyLogger::DailyLogger()
{
    QMutexLocker locker(&mutex_);
    OpenLogFile(QDate::currentDate());
}

DailyLogger::~DailyLogger() { Release(); }

void DailyLogger::Install() { qInstallMessageHandler(DailyLogger::MessageHandler); }

void DailyLogger::Release()
{
    QMutexLocker locker(&mutex_);

    if (log_stream_.device()) {
        log_stream_.flush();
    }

    if (file_.isOpen()) {
        file_.close();
    }

    is_released_ = true;
}

void DailyLogger::MessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    DailyLogger::Instance().HandleMessage(type, context, msg);
}

void DailyLogger::HandleMessage(QtMsgType type, const QMessageLogContext&, const QString& msg)
{
    QMutexLocker locker(&mutex_);

    const QDate today = QDate::currentDate();
    if (current_date_ != today) {
        OpenLogFile(today);
    }

    if (is_released_ || !file_.isOpen())
        return;

    static const char* levels[] = { "DEBUG", "INFO", "WARNING", "CRITICAL", "FATAL" };
    const char* level = levels[type];

    const QString date_time { QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") };

    formatted_msg_.clear();
    formatted_msg_.append(date_time).append(" [").append(level).append("] ").append(msg);

    log_stream_ << formatted_msg_ << '\n';

    if (type >= QtCriticalMsg) {
        log_stream_.flush();
    }

    locker.unlock();

    fprintf(stderr, "%s\n", formatted_msg_.toLocal8Bit().constData());
    if (type >= QtCriticalMsg) {
        fflush(stderr);
    }

    if (type == QtFatalMsg)
        abort();
}

void DailyLogger::OpenLogFile(const QDate& date)
{
    if (file_.isOpen()) {
        log_stream_.flush();
        file_.close();
    }

    const QString log_path { QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("logs") };
    const QString log_name { date.toString("yyyy-MM-dd") + ".log" };
    const QString file_name { QDir(log_path).filePath(log_name) };

    QDir().mkpath(log_path);

    file_.setFileName(file_name);
    if (file_.open(QIODevice::Append | QIODevice::Text)) {
        log_stream_.setDevice(&file_);
        formatted_msg_.reserve(512);
    }

    current_date_ = date;
}
