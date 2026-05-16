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

namespace StringConst {
inline const QString kEightDigits = QStringLiteral("+00,000,000.00");
inline const QString kFourDigits = QStringLiteral("+0,000.00");
inline const QString kEmpty = QStringLiteral("");
}

namespace NodeRef {
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

#endif // CONSTANTSTRING_H
