#include "testutils.h"

#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qradiobutton.h>

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QElapsedTimer>
#include <QLineEdit>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QRandomGenerator>
#include <QResource>
#include <QTest>
#include <QTimer>
#include <QTreeView>

namespace test_utils {

InsertArg RandomInsertArg()
{
    return {
        .name = RandomName("test_"),
        .kind = RandomInt(0, 1),
        .unit = RandomInt(0, 3),
        .direction_rule = RandomInt(0, 1) == 1,
    };
}

QString RandomName(const QString& prefix, int length)
{
    static const QString chars { QStringLiteral("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") };
    QString result { prefix };
    for (int i = 0; i != length; ++i)
        result.append(chars.at(QRandomGenerator::global()->bounded(chars.size())));
    return result;
}

int RandomInt(int min, int max) { return QRandomGenerator::global()->bounded(min, max + 1); }

QString RandomText(const QString& prefix, int length = 6) { return RandomName(prefix, length); }

void HandleEditNodeName(const QString& new_name)
{
    auto* timer = new QTimer(qApp);
    timer->setInterval(100);
    auto count = std::make_shared<int>(0);

    QObject::connect(timer, &QTimer::timeout, qApp, [timer, count, new_name] {
        if (++*count > 100) {
            qWarning() << "改名对话框未出现";
            timer->stop();
            timer->deleteLater();
            return;
        }

        auto* dlg = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (!dlg)
            return;
        timer->stop();
        timer->deleteLater();

        QPointer<QDialog> dialog { dlg };
        QPointer<QLineEdit> line { dlg->findChild<QLineEdit*>("lineName") };
        QPointer<QPushButton> ok { dlg->findChild<QPushButton*>("pBtnOk") };
        QPointer<QPushButton> cancel { dlg->findChild<QPushButton*>("pBtnCancel") };

        if (!line || !ok) {
            dialog->reject();
            return;
        }

        line->setFocus();
        line->selectAll();
        QTest::keyClicks(line.data(), new_name); // 触发 textEdited，启用 OK

        const bool confirm = RandomInt(0, 1) == 1;
        qInfo() << "[rename dialog] ->" << (confirm ? "OK" : "Cancel");

        if (confirm && ok->isEnabled())
            QTest::mouseClick(ok.data(), Qt::LeftButton);
        else if (cancel)
            QTest::mouseClick(cancel.data(), Qt::LeftButton);
        else
            dialog->reject();
    });
    timer->start();
}

void HandleDeleteBranch()
{
    auto* timer = new QTimer(qApp);
    timer->setInterval(100);
    auto count = std::make_shared<int>(0);

    QObject::connect(timer, &QTimer::timeout, qApp, [timer, count] {
        if (++*count > 100) {
            qWarning() << "删除确认框未出现";
            timer->stop();
            timer->deleteLater();
            return;
        }

        const auto windows = QApplication::topLevelWidgets();
        for (auto* widget : windows) {
            auto* dlg = qobject_cast<QMessageBox*>(widget);
            if (!dlg || !dlg->isVisible())
                continue;

            auto* yes = dlg->button(QMessageBox::Yes);
            auto* cancel = dlg->button(QMessageBox::Cancel);
            if (!yes || !cancel)
                continue;

            timer->stop();
            timer->deleteLater();

            const bool confirm = RandomInt(0, 1) == 1;
            qInfo() << "[delete branch] confirm dialog ->" << (confirm ? "Yes" : "Cancel");
            (confirm ? yes : cancel)->click();
            return;
        }
    });
    timer->start();
}

void FillLine(QDialog* dlg, const char* object_name, const QString& text)
{
    auto* line = dlg->findChild<QLineEdit*>(object_name);
    if (!line)
        return;

    line->setFocus();
    line->clear();
    QTest::keyClicks(line, text);
    emit line->editingFinished();
}

void HandleInsertNodeFIT(const InsertArg& arg)
{
    auto* timer = new QTimer(qApp);
    timer->setInterval(100);
    QObject::connect(timer, &QTimer::timeout, qApp, [timer, arg] {
        auto* dlg = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if (!dlg)
            return;
        timer->stop();
        timer->deleteLater();

        // 先看清弹出的到底是什么对话框
        qInfo() << "dialog:" << dlg->metaObject()->className();
        if (QString(dlg->metaObject()->className()) != "InsertNodeFIT") {
            dlg->reject();
            return;
        }

        if (auto* kind_btn = dlg->findChild<QRadioButton*>(arg.kind == 0 ? "rBtnBranch" : "rBtnLeaf"))
            kind_btn->click();

        if (auto* rule_btn = dlg->findChild<QRadioButton*>(arg.direction_rule ? "rBtnDICD" : "rBtnDDCI"); rule_btn && rule_btn->isEnabled())
            rule_btn->click();

        // FillLine 也要改成：找不到就 return，不要 Q_ASSERT
        FillLine(dlg, "lineName", arg.name);
        FillLine(dlg, "lineCode", arg.code);
        FillLine(dlg, "lineDescription", arg.description);

        if (auto* combo = dlg->findChild<QComboBox*>("comboUnit"); combo && combo->isEnabled())
            combo->setCurrentIndex(qMin(arg.unit, combo->count() - 1));

        QPointer<QDialog> dialog { dlg };
        QPointer<QPushButton> ok { dlg->findChild<QPushButton*>("pBtnOk") };
        QPointer<QPushButton> cancel { dlg->findChild<QPushButton*>("pBtnCancel") };

        if (!dialog)
            return;

        // 随机选择确定或取消
        const bool confirm = RandomInt(0, 1) == 1;
        qInfo() << "[dialog]" << dialog->metaObject()->className() << "->" << (confirm ? "OK" : "Cancel");

        if (confirm && ok && ok->isEnabled()) {
            QTest::mouseClick(ok.data(), Qt::LeftButton);
        } else if (cancel) {
            QTest::mouseClick(cancel.data(), Qt::LeftButton);
        } else {
            dialog->reject();
        }
    });
    timer->start();
}

QPushButton* FindButton(const QString& object_name)
{
    QWidget* active = QApplication::activeModalWidget();
    if (!active)
        active = QApplication::activeWindow();
    if (!active)
        return nullptr;

    auto* b = active->findChild<QPushButton*>(object_name);
    return (b && b->isVisible()) ? b : nullptr;
}

QPushButton* FindLoginButton()
{
    const auto tops = QApplication::topLevelWidgets();
    for (QWidget* top : tops) {
        if (!top->isVisible())
            continue;
        if (auto* b = top->findChild<QPushButton*>("pushButtonLogin"); b && b->isVisible())
            return b;
    }
    return nullptr;
}

bool Login()
{
    QPushButton* btn = nullptr;
    if (!QTest::qWaitFor([&] { return (btn = FindLoginButton()) != nullptr; }, 5000))
        return false;
    if (!btn->isEnabled())
        return false;
    QTest::mouseClick(btn, Qt::LeftButton);
    return true;
}

bool EnsureActive(QWidget* w)
{
    if (!w) {
        qWarning() << "目标窗口为空";
        return false;
    }

    for (int i = 0; i != 3; ++i) {
        w->raise();
        w->activateWindow();
        if (QTest::qWaitFor([&] { return QApplication::activeWindow() == w; }, 1000))
            return true;
    }
    qWarning() << "主窗口未能激活, active:" << QApplication::activeWindow();
    return false;
}

TableWidget* CurrentTableWidget(QWidget* w)
{
    if (!w)
        return nullptr;

    auto* tabs = w->findChild<QTabWidget*>("tabWidgetF");
    if (!tabs)
        return nullptr;

    return qobject_cast<TableWidget*>(tabs->currentWidget());
}

}