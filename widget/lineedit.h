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

#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QDateTime>
#include <QKeyEvent>
#include <QLineEdit>
#include <QRegularExpressionValidator>

#include "component/constant.h"

class LineEdit final : public QLineEdit {
public:
    explicit LineEdit(QWidget* parent = nullptr)
        : QLineEdit { parent }
    {
    }

    static inline const QRegularExpression kInputRegex { QStringLiteral("[\\p{L} ()（）\\d]*") };
    static inline const QRegularExpressionValidator kInputValidator { kInputRegex };

protected:
    void keyPressEvent(QKeyEvent* event) override
    {
        const Qt::KeyboardModifiers modifiers { event->modifiers() };

        if (event->key() == Qt::Key_Semicolon) {
            if (modifiers == Qt::ControlModifier) {
                insert(QDate::currentDate().toString(kDateFST));
                return;
            }
            if (modifiers == (Qt::ControlModifier | Qt::ShiftModifier)) {
                insert(QDateTime::currentDateTime().toString(kDateTimeFST));
                return;
            }
        }

        QLineEdit::keyPressEvent(event);
    }
};

#endif // LINEEDIT_H
