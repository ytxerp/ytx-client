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

#ifndef COLLATOR_H
#define COLLATOR_H

#include <QCollator>
#include <QLocale>

class Collator {
public:
    static QCollator& Instance()
    {
        static QCollator instance {};
        return instance;
    }

    static void SetLanguage(const QLocale& locale)
    {
        static std::once_flag flag {};
        std::call_once(flag, [&]() {
            QCollator& collator = Instance();
            collator.setLocale(locale);
            collator.setIgnorePunctuation(true);
            collator.setCaseSensitivity(Qt::CaseInsensitive);
        });
    }

    Collator(const Collator&) = delete;
    Collator& operator=(const Collator&) = delete;
    Collator(Collator&&) = delete;
    Collator& operator=(Collator&&) = delete;
};

#endif // COLLATOR_H
