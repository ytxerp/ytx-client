#include "about.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

#include "ui_about.h"

About::About(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::About)
{
    ui->setupUi(this);
    IniDialog();
}

About::~About() { delete ui; }

void About::OpenResourceHtml(const QString& file_name)
{
    // Load the HTML file from Qt resource system
    QFile file(QString(":/policies/policies/%1").arg(file_name));
    if (!file.exists())
        return; // If file doesn't exist, do nothing

    // Determine a temporary file path
    const QString temp_path { QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QDir::separator() + "ytx_" + file_name };

    // Remove existing temp file to ensure fresh copy
    if (QFile::exists(temp_path)) {
        QFile::remove(temp_path);
    }

    // Copy resource file to temporary location
    if (file.open(QIODevice::ReadOnly)) {
        QFile temp_file(temp_path);
        if (temp_file.open(QIODevice::WriteOnly)) {
            temp_file.write(file.readAll());
            temp_file.close();
            file.close();

            qDebug() << "Temp file path:" << temp_file.fileName();

            // Open the temporary file in the system default browser
            QDesktopServices::openUrl(QUrl::fromLocalFile(temp_path));
        }
    }
}

void About::IniDialog()
{
    static const QString html
        = QString(R"(
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
<html>
<body>
    <p><b>YTX v0.3.5</b><br>© 2026 YTX</p>
    <p>%1</p>
    <p>%2</p>
    <p><a href="privacy_policy.html">%3</a> | <a href="terms_of_service.html">%4</a></p>
</body>
</html>
)")
              .arg(tr("YTX is an enterprise resource planning system designed to help businesses manage operations efficiently."),
                  tr("Please review our Privacy Policy and Terms of Service for full details."), tr("Privacy Policy"), tr("Terms of Service"));

    ui->textBrowser->setHtml(html);

    // Prevent QTextBrowser from opening links itself
    ui->textBrowser->setOpenLinks(false);

    // Connect QTextBrowser anchor clicks to a lambda
    connect(ui->textBrowser, &QTextBrowser::anchorClicked, this, [](const QUrl& url) {
        About::OpenResourceHtml(url.toString()); // static function call
    });
}
