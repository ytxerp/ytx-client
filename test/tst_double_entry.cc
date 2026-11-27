#include <QtWidgets/qcolordialog.h>
#include <QtWidgets/qcombobox.h>

#include <QLineEdit>
#include <QResource>
#include <QTest>

#include "mainwindow/mainwindow.h"
#include "testutils.h"
#include "utils/mainutils.h"

using namespace test_utils;

class TestDoubleEntry : public QObject {
    Q_OBJECT

private slots:
    void initTestCase()
    {
        resource_file_ = QStringLiteral(PROJECT_SOURCE_DIR) + "/resource/resource.brc";

        QVERIFY2(QFile::exists(resource_file_), qPrintable("找不到: " + resource_file_));
        QVERIFY2(QResource::registerResource(resource_file_), qPrintable("注册失败: " + resource_file_));

        qApp->setStyle(QStringLiteral("Fusion"));
        qApp->setAttribute(Qt::AA_DontUseNativeDialogs);
        qApp->setQuitOnLastWindowClosed(false);
        utils::SetAppIcon(*qApp);
        QCoreApplication::setApplicationName("YTX");

        w_ = new MainWindow();
        w_->show();
        QVERIFY(QTest::qWaitForWindowExposed(w_));

        QTest::qWait(1000);

        QVERIFY2(Login(), "登录框未出现或登录按钮不可用");

        // 登录框关闭
        QVERIFY(QTest::qWaitFor([] { return FindLoginButton() == nullptr; }, 10000));

        // 树同步完成（finance 树有数据）
        QVERIFY(QTest::qWaitFor(
            [this] {
                auto* tree = CurrentTree(w_);
                return tree && tree->model() && tree->model()->rowCount() > 0;
            },
            15000));
    }

    void cleanupTestCase()
    {
        w_->hide();
        QTest::qWait(100);
        delete w_;
        w_ = nullptr;
        QResource::unregisterResource(resource_file_);
    }

    void manual()
    {
        w_->show();
        w_->raise();
        w_->activateWindow();

        while (w_->isVisible())
            QTest::qWait(200);
    }

private:
    MainWindow* w_ { nullptr };
    QString resource_file_ {};
};

QTEST_MAIN(TestDoubleEntry)
#include "tst_double_entry.moc"