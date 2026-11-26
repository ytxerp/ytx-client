#include <QResource>
#include <QTest>

#include "mainwindow/mainwindow.h"
#include "testutils.h"
#include "utils/mainutils.h"

using namespace test_utils;

class TestFinance : public QObject {
    Q_OBJECT

private:
    MainWindow* w_ { nullptr };
    QString resource_file_ {};

private:
    QTreeView* CurrentTree()
    {
        EnsureActive();
        auto* tabs = w_->findChild<QTabWidget*>("tabWidgetF");
        return tabs ? tabs->currentWidget()->findChild<QTreeView*>() : nullptr;
    }

    bool EnsureActive()
    {
        for (int i = 0; i != 3; ++i) {
            w_->raise();
            w_->activateWindow();
            if (QTest::qWaitFor([&] { return QApplication::activeWindow() == w_; }, 1000))
                return true;
        }
        qWarning() << "主窗口未能激活, active:" << QApplication::activeWindow();
        return false;
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

    bool DoInsert()
    {
        EnsureActive();

        auto* act = w_->findChild<QAction*>("actionInsertNode");
        auto* tree = CurrentTree();
        if (!act || !act->isEnabled() || !tree)
            return false;

        const InsertArg arg { RandomInsertArg() };
        qInfo() << "[insert]" << arg.name << "kind" << arg.kind << "rule" << arg.direction_rule << "unit" << arg.unit;

        HandleInsertNodeFIT(arg);
        act->trigger();
        return true;
    }

    bool DoRename()
    {
        EnsureActive();

        auto* act = w_->findChild<QAction*>("actionRename");
        auto* tree = CurrentTree();
        if (!act || !tree)
            return false;

        auto* model = tree->model();
        const QModelIndexList found = model->match(model->index(0, 0), Qt::DisplayRole, "test", -1, Qt::MatchStartsWith | Qt::MatchRecursive);
        if (found.isEmpty())
            return false;

        tree->setCurrentIndex(found.at(RandomInt(0, static_cast<int>(found.size()) - 1)));

        w_->activateWindow();
        w_->raise();
        if (!act->isEnabled())
            return false;

        const QString new_name { RandomName("test", 8) };
        qInfo() << "[rename]" << tree->currentIndex().data().toString() << "->" << new_name;

        HandleEditNodeName(new_name);
        act->trigger();
        return true;
    }

    bool DoDelete()
    {
        if (!EnsureActive())
            return false;

        auto* tree = CurrentTree();
        if (!tree)
            return false;

        auto* model = qobject_cast<TreeModel*>(tree->model()); // 不用 static_cast，代理模型时返回 nullptr
        if (!model)
            return false;

        auto* root = model->GetNodeByIndex(QModelIndex());
        if (!root)
            return false;

        // 收集所有 test 节点，只保存 id 和 kind，不跨步骤持有 Node*
        struct Candidate {
            QUuid id {};
            bool is_branch {};
        };
        QList<Candidate> candidates;

        std::function<void(Node*)> collect = [&](Node* parent) {
            for (auto* child : parent->children) {
                if (child->IsValid() && child->name.startsWith("test", Qt::CaseInsensitive))
                    candidates.append({ child->id, NodeKind(child->kind) == NodeKind::kBranch });
                collect(child);
            }
        };
        collect(root);

        if (candidates.isEmpty())
            return false; // 没有可删的，属于正常情况

        // 随机选一个，而不是永远删第一个
        const auto pick = candidates.at(RandomInt(0, static_cast<int>(candidates.size()) - 1));

        const QModelIndex idx { model->GetIndex(pick.id) };
        if (!idx.isValid())
            return false;
        tree->setCurrentIndex(idx);

        qInfo() << "[delete]" << idx.data().toString() << (pick.is_branch ? "branch" : "leaf");

        // 分支会弹确认框（非阻塞 show），先安排好处理再触发
        if (pick.is_branch)
            HandleDeleteBranch();

        QMetaObject::invokeMethod(w_, "on_actionDelete_triggered", Qt::DirectConnection);
        return true; // 等待时间交给 monkey() 的随机间隔
    }

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
                auto* tree = CurrentTree();
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

    void monkey()
    {
        QTRY_VERIFY_WITH_TIMEOUT(FindLoginButton() == nullptr, 10000);
        QTRY_VERIFY_WITH_TIMEOUT(w_->findChild<QAction*>("actionInsertNode")->isEnabled(), 15000);

        const std::vector<std::pair<const char*, std::function<bool()>>> actions {
            { "insert", [this] { return DoInsert(); } },
            { "rename", [this] { return DoRename(); } },
            { "delete", [this] { return DoDelete(); } },
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

    void deleteTestNodes()
    {
        for (int i = 0; i != 20; ++i) {
            qInfo() << "[T] delete round" << (i + 1) << "/ 20";

            if (!DoDelete()) {
                qInfo() << "[T] no more test nodes";
                break;
            }

            // 等服务器响应和树刷新；分支确认框也在这段时间内被自动处理
            QTest::qWait(1000);
            QVERIFY(w_->isVisible());

            // 兜底：清理残留模态窗口
            while (auto* m = QApplication::activeModalWidget()) {
                qWarning() << "[T] 残留模态窗口:" << m->metaObject()->className();
                m->close();
            }
        }

        QTest::qWait(500); // 让最后一次响应回来
    }

    void manual()
    {
        w_->resize(1400, 900);
        w_->show();
        w_->raise();
        w_->activateWindow();

        while (w_->isVisible())
            QTest::qWait(200);
    }
};

QTEST_MAIN(TestFinance)
#include "tst_finance.moc"