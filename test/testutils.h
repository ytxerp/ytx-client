#pragma once

#include <QtWidgets/qtreeview.h>

#include <QAbstractItemModel>
#include <QPushButton>
#include <QRandomGenerator>
#include <QString>

struct InsertArg {
    QString name {};
    QString code {};
    QString description {};
    int kind {};
    int unit {};
    bool direction_rule {};
};

namespace test_utils {

// 随机
int RandomInt(int min, int max);
QString RandomName(const QString& prefix = QStringLiteral("test"), int length = 8);
InsertArg RandomInsertArg();

// 查找 / 等待
QPushButton* FindLoginButton();
QPushButton* FindButton(const QString& object_name);
bool HasNode(QAbstractItemModel* model, const QString& name);

// 对话框自动处理（先调用，再触发会弹框的操作）
void HandleInsertNodeFIT(const InsertArg& arg);
void HandleEditNodeName(const QString& new_name);
void HandleDeleteBranch();

QTreeView* VisibleTree(QWidget* root);

void FillLine(QDialog* dlg, const char* object_name, const QString& text);
void HandleInsertNodeFIT(const InsertArg& arg);
bool Login();
bool EnsureActive(QWidget* w);

QTreeView* CurrentTree(QWidget* w);

}