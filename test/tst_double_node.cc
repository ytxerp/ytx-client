#include <QtWidgets/qcolordialog.h>
#include <QtWidgets/qcombobox.h>

#include <QLineEdit>
#include <QResource>
#include <QTest>

#include "mainwindow/mainwindow.h"
#include "testutils.h"
#include "utils/mainutils.h"
#include "utils/nodeutils.h"

using namespace test_utils;

class TestDoubleNode : public QObject {
    Q_OBJECT

private:
    MainWindow* w_ { nullptr };
    QString resource_file_ {};

private:
    bool DoInsert()
    {
        EnsureActive(w_);

        const InsertArg arg { RandomInsertArg() };
        HandleInsertNodeFIT(arg);
        w_->on_actionInsertNode_triggered();
        return true;
    }

    bool DoRename()
    {
        EnsureActive(w_);

        auto* act = w_->findChild<QAction*>("actionRename");
        auto tree = w_->sc_->tree_view;
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

    bool DoChangeDirectionRule()
    {
        if (!EnsureActive(w_))
            return false;

        auto tree = w_->sc_->tree_view;
        if (!tree)
            return false;

        auto model = w_->sc_->tree_model;
        if (!model)
            return false;

        const int col = node::DirectionRuleColumn(Section::kFinance);
        if (col < 0)
            return false;

        auto* root = model->GetNodeByIndex(QModelIndex());
        if (!root)
            return false;

        QList<QUuid> candidates;
        std::function<void(Node*)> collect = [&](Node* parent) {
            for (auto* child : std::as_const(parent->children)) {
                if (child->IsValid() && child->name.startsWith("test", Qt::CaseInsensitive))
                    candidates.append(child->id);
                collect(child);
            }
        };
        collect(root);

        if (candidates.isEmpty())
            return false;

        const QUuid target_id = candidates.at(RandomInt(0, static_cast<int>(candidates.size()) - 1));
        const QModelIndex row_idx = model->GetIndex(target_id);
        if (!row_idx.isValid())
            return false;

        const QModelIndex target_idx = model->index(row_idx.row(), col, row_idx.parent());
        if (!target_idx.isValid())
            return false;

        // 选中目标列索引
        tree->setCurrentIndex(target_idx);

        const QRect rect = tree->visualRect(target_idx);
        if (!rect.isValid() || rect.width() <= 0 || rect.height() <= 0)
            return false;

        const QPoint pos = rect.center();
        const QString before_val = target_idx.data(Qt::DisplayRole).toString();

        // 点击并双击该列单元格
        QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, pos);
        QTest::qWait(50);
        QTest::mouseDClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, pos);
        QTest::qWait(100);

        const QString after_val = model->index(row_idx.row(), col, row_idx.parent()).data(Qt::DisplayRole).toString();
        qInfo() << "[direction_rule]" << row_idx.data().toString() << before_val << "->" << after_val;
        return true;
    }

    bool DoUpdateDesc()
    {
        if (!EnsureActive(w_))
            return false;

        auto tree = w_->sc_->tree_view;
        if (!tree)
            return false;

        auto model = w_->sc_->tree_model;
        if (!model)
            return false;

        const int col = node::DescriptionColumn(Section::kFinance);
        if (col < 0)
            return false;

        auto* root = model->GetNodeByIndex(QModelIndex());
        if (!root)
            return false;

        QList<QUuid> candidates;
        std::function<void(Node*)> collect = [&](Node* parent) {
            for (auto* child : parent->children) {
                if (child->IsValid() && child->name.startsWith("test", Qt::CaseInsensitive))
                    candidates.append(child->id);
                collect(child);
            }
        };
        collect(root);

        if (candidates.isEmpty())
            return false;

        const QUuid target_id = candidates.at(RandomInt(0, static_cast<int>(candidates.size()) - 1));
        const QModelIndex row_idx = model->GetIndex(target_id);
        if (!row_idx.isValid())
            return false;

        const QModelIndex desc_idx = model->index(row_idx.row(), col, row_idx.parent());
        if (!desc_idx.isValid())
            return false;

        // 选中对应节点，让详情面板同步到该节点
        tree->setCurrentIndex(row_idx);
        QTest::qWait(50);

        // 按实际控件对象名替换（如 QLineEdit）
        const QRect rect = tree->visualRect(desc_idx);
        if (!rect.isValid() || rect.width() <= 0 || rect.height() <= 0)
            return false;

        const QPoint pos = rect.center();
        const QString before_desc = desc_idx.data(Qt::DisplayRole).toString();
        const QString next_desc = RandomName("desc", 8);

        // 1. 单击选中后双击打开 QLineEdit 代理
        QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, pos);
        QTest::qWait(50);
        QTest::mouseDClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, pos);

        // 2. 等待代理创建并聚焦 QLineEdit
        QLineEdit* line = nullptr;
        if (!QTest::qWaitFor(
                [&] {
                    line = qobject_cast<QLineEdit*>(QApplication::focusWidget());
                    return line != nullptr && line->isVisible();
                },
                1000)) {
            return false;
        }

        // 3. 替换内容并回车提交（触发后续防抖计时）
        line->selectAll();
        QTest::keyClick(line, Qt::Key_Backspace);
        QTest::keyClicks(line, next_desc);
        QTest::keyClick(line, Qt::Key_Return);

        QTest::qWait(50);
        qInfo() << "[desc]" << row_idx.data().toString() << before_desc << "->" << next_desc;
        return true;
    }

    bool DoChangeColor()
    {
        if (!EnsureActive(w_))
            return false;

        auto tree = w_->sc_->tree_view;
        if (!tree)
            return false;

        auto model = w_->sc_->tree_model;
        if (!model)
            return false;

        const int col = node::ColorColumn(Section::kFinance);
        if (col < 0)
            return false;

        auto* root = model->GetNodeByIndex(QModelIndex());
        if (!root)
            return false;

        QList<QUuid> candidates;
        std::function<void(Node*)> collect = [&](Node* parent) {
            for (auto* child : std::as_const(parent->children)) {
                if (child->IsValid() && child->name.startsWith("test", Qt::CaseInsensitive))
                    candidates.append(child->id);
                collect(child);
            }
        };
        collect(root);

        if (candidates.isEmpty())
            return false;

        const QUuid target_id = candidates.at(RandomInt(0, static_cast<int>(candidates.size()) - 1));
        const QModelIndex row_idx = model->GetIndex(target_id);
        if (!row_idx.isValid())
            return false;

        const QModelIndex color_idx = model->index(row_idx.row(), col, row_idx.parent());
        if (!color_idx.isValid())
            return false;

        tree->scrollTo(color_idx);
        tree->setCurrentIndex(color_idx);

        const QRect rect = tree->visualRect(color_idx);
        if (!rect.isValid() || rect.width() <= 0 || rect.height() <= 0)
            return false;

        const QPoint pos = rect.center();
        const QString before_color = color_idx.data().toString();
        const QColor target_color(RandomInt(0, 255), RandomInt(0, 255), RandomInt(0, 255), RandomInt(128, 255));
        const bool accept_dialog = (RandomInt(0, 99) < 70);
        QPointer<TestDoubleNode> guard(this);

        // 双击前提前排好动作：等原生 QColorDialog 弹出后随机决定确认或取消
        QTimer::singleShot(50, this, [guard, target_color, accept_dialog] {
            if (!guard)
                return;

            if (auto* dlg = qobject_cast<QColorDialog*>(QApplication::activeModalWidget())) {
                if (accept_dialog) {
                    dlg->setCurrentColor(target_color);
                    dlg->accept();
                } else {
                    dlg->reject();
                }
            }
        });

        QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, pos);
        QTest::qWait(50);
        QTest::mouseDClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, pos);
        QTest::qWait(100);

        const QString after_color = model->index(row_idx.row(), col, row_idx.parent()).data().toString();
        qInfo() << "[color]" << row_idx.data().toString() << before_color << "->" << after_color << (accept_dialog ? "(accepted)" : "(rejected)");
        return true;
    }

    bool DoDelete()
    {
        if (!EnsureActive(w_))
            return false;

        auto tree = w_->sc_->tree_view;
        if (!tree)
            return false;

        auto model = w_->sc_->tree_model;
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

    bool DoClearColor()
    {
        if (!EnsureActive(w_))
            return false;

        auto tree = w_->sc_->tree_view;
        if (!tree)
            return false;

        auto model = w_->sc_->tree_model;
        if (!model)
            return false;

        const int col = node::ColorColumn(Section::kFinance);
        if (col < 0)
            return false;

        auto* root = model->GetNodeByIndex(QModelIndex());
        if (!root)
            return false;

        // 优先挑已有颜色的 test* 节点，便于验证清空效果；无则退化为所有 test* 节点
        QList<Node*> colored_candidates;
        QList<Node*> all_candidates;
        std::function<void(Node*)> collect = [&](Node* parent) {
            for (auto* child : std::as_const(parent->children)) {
                if (child->IsValid() && child->name.startsWith("test", Qt::CaseInsensitive)) {
                    all_candidates.append(child);
                    if (!child->color.isEmpty())
                        colored_candidates.append(child);
                }
                collect(child);
            }
        };
        collect(root);

        const auto& pool = !colored_candidates.isEmpty() ? colored_candidates : all_candidates;
        if (pool.isEmpty())
            return false;

        Node* target = pool.at(RandomInt(0, static_cast<int>(pool.size()) - 1));
        const QModelIndex row_idx = model->GetIndex(target->id);
        if (!row_idx.isValid())
            return false;

        const QModelIndex color_idx = model->index(row_idx.row(), col, row_idx.parent());
        if (!color_idx.isValid())
            return false;

        tree->scrollTo(color_idx);
        tree->setCurrentIndex(color_idx);
        tree->setFocus(Qt::OtherFocusReason);
        QTest::qWait(50);

        const QString before_color = target->color;
        const QString node_name = target->name;

        if (w_) {
            if (auto* act = w_->findChild<QAction*>("actionClearColor")) {
                act->trigger();
            } else {
                QMetaObject::invokeMethod(w_, "on_actionClearColor_triggered", Qt::DirectConnection);
            }
        }
        QTest::qWait(100);

        qInfo() << "[clear_color]" << node_name << before_color << "->" << target->color;
        return true;
    }

    bool DoOpenNodeTab()
    {
        if (!EnsureActive(w_))
            return false;

        auto tree = w_->sc_->tree_view;
        if (!tree)
            return false;

        auto model = w_->sc_->tree_model;
        if (!model)
            return false;

        auto* root = model->GetNodeByIndex(QModelIndex());
        if (!root)
            return false;

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

        if (candidates.isEmpty())
            return false;

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
        if (!rect.isValid() || rect.width() <= 0 || rect.height() <= 0)
            return false;

        const QPoint pos = rect.center();
        const QString node_name = name_idx.data(Qt::DisplayRole).toString();

        QTest::mouseClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, pos);
        QTest::qWait(50);
        QTest::mouseDClick(tree->viewport(), Qt::LeftButton, Qt::NoModifier, pos);
        QTest::qWait(100);

        if (w_) {
            if (auto* tab_widget = w_->findChild<QTabWidget*>("tabWidgetF")) {
                if (tab_widget->count() > 0) {
                    tab_widget->setCurrentIndex(0);
                    if (auto* page = tab_widget->widget(0))
                        tab_widget->setCurrentWidget(page);
                }
            }
        }
        EnsureActive(w_);
        tree->setFocus(Qt::OtherFocusReason);
        tree->setCurrentIndex(name_idx);
        QTest::qWait(50);

        qInfo() << "[open_tab]" << node_name << "tree_focus:" << tree->hasFocus();
        return true;
    }

    bool DoMoveNode()
    {
        if (!EnsureActive(w_))
            return false;

        auto tree = w_->sc_->tree_view;
        if (!tree)
            return false;

        auto model = w_->sc_->tree_model;
        if (!model)
            return false;

        auto* root = model->GetNodeByIndex(QModelIndex());
        if (!root)
            return false;

        QList<Node*> leaves;
        QList<Node*> branches;
        std::function<void(Node*)> collect = [&](Node* parent) {
            for (auto* child : std::as_const(parent->children)) {
                if (child->IsValid() && child->name.startsWith("test", Qt::CaseInsensitive)) {
                    if (child->kind == NodeKind::kLeaf)
                        leaves.append(child);
                    else if (child->kind == NodeKind::kBranch)
                        branches.append(child);
                }
                collect(child);
            }
        };
        collect(root);

        if (leaves.isEmpty() || branches.isEmpty())
            return false;

        Node* leaf = leaves.at(RandomInt(0, static_cast<int>(leaves.size()) - 1));
        Node* target_branch = nullptr;

        // 挑一个不是当前父节点的 branch，减少无效拖拽
        QList<Node*> valid_branches;
        for (auto* b : std::as_const(branches)) {
            if (b != leaf->parent)
                valid_branches.append(b);
        }
        if (valid_branches.isEmpty())
            return false;

        target_branch = valid_branches.at(RandomInt(0, static_cast<int>(valid_branches.size()) - 1));

        const QModelIndex src_row = model->GetIndex(leaf->id);
        const QModelIndex dst_row = model->GetIndex(target_branch->id);
        if (!src_row.isValid() || !dst_row.isValid())
            return false;

        const QModelIndex src_idx = model->index(src_row.row(), 0, src_row.parent());
        const QModelIndex dst_idx = model->index(dst_row.row(), 0, dst_row.parent());
        if (!src_idx.isValid() || !dst_idx.isValid())
            return false;

        tree->expand(dst_idx);
        tree->scrollTo(dst_idx, QAbstractItemView::PositionAtCenter);
        tree->setCurrentIndex(src_idx);
        QApplication::processEvents();

        const QUuid old_parent_id = leaf->parent ? leaf->parent->id : QUuid();

        // ===== 新增:直接走 model 的拖放逻辑 =====
        QMimeData* mime = model->mimeData({ src_idx });
        if (!mime) {
            qWarning() << "[move] mimeData returned null";
            return false;
        }

        const bool can_drop = model->canDropMimeData(mime, Qt::MoveAction, -1, 0, dst_idx);

        if (can_drop)
            model->dropMimeData(mime, Qt::MoveAction, -1, 0, dst_idx);
        delete mime;

        QApplication::processEvents();
        // ===== 新增结束 =====

        const QUuid new_parent_id = leaf->parent ? leaf->parent->id : QUuid();
        qInfo() << "[move]" << leaf->name << "-> branch" << target_branch->name << "parent_changed:" << (old_parent_id != new_parent_id);
        return (old_parent_id != new_parent_id);
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

        QVERIFY2(Login(), "登录框未出现或登录按钮不可用");
        QVERIFY(QTest::qWaitFor([this] { return w_->tree_synced; }, 15000));
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
        QTRY_VERIFY_WITH_TIMEOUT(w_->findChild<QAction*>("actionInsertNode")->isEnabled(), 15000);

        const std::vector<std::pair<const char*, std::function<bool()>>> actions {
            { "insert", [this] { return DoInsert(); } },
            { "rename", [this] { return DoRename(); } },
            { "delete", [this] { return DoDelete(); } },
            { "direction_rule", [this] { return DoChangeDirectionRule(); } },
            { "desc", [this] { return DoUpdateDesc(); } },
            { "open_tab", [this] { return DoOpenNodeTab(); } },
            { "color", [this] { return DoChangeColor(); } },
            { "clear_color", [this] { return DoClearColor(); } },
            { "move node", [this] { return DoMoveNode(); } },

        };

        const int steps = qEnvironmentVariable("YTX_STEPS", "50").toInt();
        const int min_ms = qEnvironmentVariable("YTX_MIN_MS", "100").toInt();
        const int max_ms = qEnvironmentVariable("YTX_MAX_MS", "1500").toInt();

        for (int i = 0; i != steps; ++i) {
            const auto& [name, fn] = actions[static_cast<size_t>(RandomInt(0, static_cast<int>(actions.size()) - 1))];
            const int delay = RandomInt(min_ms, max_ms);
            qInfo() << "step" << i << name << "next delay" << delay;

            fn();

            QTest::qWait(delay);
            QVERIFY(w_->isVisible());

            while (auto* m = QApplication::activeModalWidget()) {
                qWarning() << "残留模态窗口:" << m->metaObject()->className();
                m->close();
            }
        }
    }

    void testInsertNodes()
    {
        for (int i = 0; i != 20; ++i) {
            qInfo() << "[T] insert round" << (i + 1) << "/ 20";

            if (!DoInsert()) {
                qWarning() << "[T] insert skipped or unavailable at round" << (i + 1);
                break;
            }

            // 等待服务端响应、树刷新以及可能的弹窗处理
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

    void testdeleteNodes()
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
        w_->show();
        w_->raise();
        w_->activateWindow();

        while (w_->isVisible())
            QTest::qWait(200);
    }
};

QTEST_MAIN(TestDoubleNode)
#include "tst_double_node.moc"