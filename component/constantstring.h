/*
 * Copyright (C) 2023 YTX
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

#ifndef CONSTANTSTRING_H
#define CONSTANTSTRING_H

#include <QString>

namespace string_const {
inline const QString kEightDigits = QStringLiteral("+00,000,000.00");
inline const QString kFourDigits = QStringLiteral("+0,000.00");
inline const QString kEmpty = QStringLiteral("");
}

namespace node_ref {
// Node is referenced by linked nodes within the same section
inline const QString kWithin = QStringLiteral("within");
// Internal inventory node is referenced by partner, sale, or purchase
inline const QString kInventoryInt = QStringLiteral("inventory_int");
// External inventory node is referenced by partner
inline const QString kInventoryExt = QStringLiteral("inventory_ext");
// Partner node (customer/vendor) is referenced by sale or purchase
inline const QString kPartnerCV = QStringLiteral("partner_cv");
// Partner node (employee) is referenced by sale or purchase
inline const QString kPartnerEmp = QStringLiteral("partner_emp");
// Sale or purchase order is already settled
inline const QString kOrder = QStringLiteral("order");
}

namespace section_heat {
inline const QString kMinOrderCount = QStringLiteral("min_order_count");
inline const QString kMinInventoryDiversity = QStringLiteral("min_inventory_diversity");
inline const QString kMinActiveMonths = QStringLiteral("min_active_months");
inline const QString kMinPartnerCount = QStringLiteral("min_partner_count");
}

namespace balance_sheet {
inline const QString kAssetId = QStringLiteral("asset_id");
inline const QString kLiabilityId = QStringLiteral("liability_id");
inline const QString kEquityId = QStringLiteral("equity_id");
}

namespace income_statement {
inline const QString kIncomeId = QStringLiteral("income_id");
inline const QString kExpenseId = QStringLiteral("expense_id");
inline const QString kNetProfit = QStringLiteral("net_profit");
}

#endif // CONSTANTSTRING_H
