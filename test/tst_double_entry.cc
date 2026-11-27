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

    void monkey()
    {
        QTRY_VERIFY_WITH_TIMEOUT(FindLoginButton() == nullptr, 10000);
        QTRY_VERIFY_WITH_TIMEOUT(w_->findChild<QAction*>("actionInsertNode")->isEnabled(), 15000);

        const std::vector<std::pair<const char*, std::function<bool()>>> actions {
            { "open_tab", [this] { return DoOpenNodeTab(); } },
        };

        const int steps = qEnvironmentVariable("YTX_STEPS", "50").toInt();
        const int min_ms = qEnvironmentVariable("YTX_MIN_MS", "100").toInt();
        const int max_ms = qEnvironmentVariable("YTX_MAX_MS", "1500").toInt();

        for (int i = 0; i != steps; ++i) {
            const auto& [name, fn] = actions[static_cast<size_t>(RandomInt(0, static_cast<int>(actions.size()) - 1))];
            const int delay = RandomInt(min_ms, max_ms);
            qInfo() << "step" << i << name << "next delay" << delay;

            fn(); // 返回 false 表示这一步无事可做，属于正常情况

            QTest::qWait(delay);
            QVERIFY(w_->isVisible());

            // 兜底：关掉残留的模态窗口，并提示出来
            while (auto* m = QApplication::activeModalWidget()) {
                qWarning() << "残留模态窗口:" << m->metaObject()->className();
                m->close();
            }
        }
    }

    void openTab()
    {
        QVERIFY(EnsureActive(w_));
        QVERIFY2(DoOpenNodeTab(), "DoOpenNodeTab 返回 false，见上方 [open_tab] 日志");
        QTest::qWait(500);
    }

    void appendEntry()
    {
        QVERIFY(EnsureActive(w_));
        QVERIFY(DoAppendEntry());
        QTest::qWait(500);
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
    bool DoOpenNodeTab()
    {
        if (!EnsureActive(w_))
            return false;

        auto* tree = CurrentTree(w_);
        if (!tree) {
            qWarning() << "[tree] invalid";
            return false;
        }

        auto* model = qobject_cast<TreeModel*>(tree->model());
        if (!model) {
            qWarning() << "[model] invalid";
            return false;
        }

        auto* root = model->GetNodeByIndex(QModelIndex());
        if (!root) {
            qWarning() << "[root] invalid";
            return false;
        }

        QList<QUuid> candidates;
        std::function<void(Node*)> collect = [&](Node* parent) {
            for (auto* child : parent->children) {
                if (child->IsValid() && child->kind == NodeKind::kLeaf && child->name.startsWith("test", Qt::CaseInsensitive)) {
                    candidates.append(child->id);
                }
                collect(child);
            }
        };
        collect(root);

        if (candidates.isEmpty()) {
            qWarning() << "[open_tab] no leaf candidates";
            return false;
        }

        const QUuid target_id = candidates.at(RandomInt(0, static_cast<int>(candidates.size()) - 1));
        const QModelIndex row_idx = model->GetIndex(target_id);
        if (!row_idx.isValid())
            return false;

        const QModelIndex name_idx = model->index(row_idx.row(), 0, row_idx.parent());
        if (!name_idx.isValid())
            return false;

        tree->scrollTo(name_idx);
        tree->setCurrentIndex(name_idx);

        const QRect rect = tree->visualRect(name_idx);
        if (!rect.isValid() || rect.width() <= 0 || rect.height() <= 0) {
            qWarning() << "[open_tab] invalid rect" << rect;
            return false;
        }

        const QPoint pos = rect.center();
        const QString node_name = name_idx.data(Qt::DisplayRole).toString();

        QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, pos);
        QTest::qWait(50);
        QTest::mouseDClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, pos);
        QTest::qWait(500);

        qInfo() << "[open_tab]" << node_name;

        return true;
    }

    bool DoAppendEntry()
    {
        auto* act = w_->findChild<QAction*>("actionAppendEntry");
        if (!act || !act->isEnabled())
            return false;

        act->trigger();
        QTest::qWait(50);

        auto* act_jmp = w_->findChild<QAction*>("actionJumpEntry");
        if (!act_jmp || !act_jmp->isEnabled())
            return false;

        act_jmp->trigger();
        QTest::qWait(50);

        QComboBox* combo = nullptr;
        if (!QTest::qWaitFor(
                [&] {
                    combo = qobject_cast<QComboBox*>(QApplication::focusWidget());
                    if (!combo)
                        if (auto* fw = QApplication::focusWidget())
                            combo = fw->parentWidget() ? qobject_cast<QComboBox*>(fw->parentWidget()) : nullptr;
                    return combo != nullptr && combo->isVisible();
                },
                1000)) {
            qWarning() << "[jump] combobox 未出现";
            return false;
        }

        const int count = combo->count();
        if (count <= 0)
            return false;

        const int idx = RandomInt(0, count - 1);
        combo->setCurrentIndex(idx);
        qInfo() << "[jump] select" << idx << combo->itemText(idx);

        // 提交编辑：回车（若代理在 activated/currentIndexChanged 时已自动提交，回车也无害）
        QTest::keyClick(combo, Qt::Key_Return);
        QTest::qWait(50);

        return true;
    }

private:
    MainWindow* w_ { nullptr };
    QString resource_file_ {};
};

QTEST_MAIN(TestDoubleEntry)
#include "tst_double_entry.moc"