#pragma once

#include <QFlags>
#include <QString>
#include <span>

namespace section {

enum class Permission : quint64 {
    // Finance: bit 0 ~ 7
    kFinanceReadOnly = quint64 { 0b01 },
    kFinanceReadWrite = quint64 { 0b11 },

    // Task: bit 8 ~ 15
    kTaskReadOnly = quint64 { 0b01 } << 8,
    kTaskReadWrite = quint64 { 0b11 } << 8,

    // Inventory: bit 16 ~ 23
    kInventoryReadOnly = quint64 { 0b01 } << 16,
    kInventoryReadWrite = quint64 { 0b11 } << 16,

    // Partner: bit 24 ~ 31
    kPartnerReadOnly = quint64 { 0b01 } << 24,
    kPartnerReadWrite = quint64 { 0b11 } << 24,

    // Sale: bit 32 ~ 46
    kSaleReadOnly = quint64 { 0b01 } << 32,
    kSaleReadWrite = quint64 { 0b11 } << 32,
    kSaleRelease = quint64 { 1 } << 34,
    kSaleUnrelease = quint64 { 1 } << 35,
    kSaleSettle = quint64 { 1 } << 36,
    kSaleUnsettle = quint64 { 1 } << 37,

    // Purchase: bit 47 ~ 61
    kPurchaseReadOnly = quint64 { 0b01 } << 47,
    kPurchaseReadWrite = quint64 { 0b11 } << 47,
    kPurchaseRelease = quint64 { 1 } << 49,
    kPurchaseUnrelease = quint64 { 1 } << 50,
    kPurchaseSettle = quint64 { 1 } << 51,
    kPurchaseUnsettle = quint64 { 1 } << 52,

    // bit 62 ~ 63: Permanently reserved. Do not use.
};

Q_DECLARE_FLAGS(Permissions, Permission)
Q_DECLARE_OPERATORS_FOR_FLAGS(Permissions)

struct PermissionItem {
    Permission permission {};
    QString text {};
};

std::span<const PermissionItem> PermissionItems();
QString PermissionsDisplay(Permissions permissions);
}
