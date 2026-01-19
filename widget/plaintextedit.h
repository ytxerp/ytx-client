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

#ifndef PLAINTEXTEDIT_H
#define PLAINTEXTEDIT_H

#include <QDateTime>
#include <QKeyEvent>
#include <QPlainTextEdit>

#include "component/constant.h"

class PlainTextEdit final : public QPlainTextEdit {
public:
    explicit PlainTextEdit(QWidget* parent = nullptr)
        : QPlainTextEdit { parent }
    {
        setUndoRedoEnabled(true);
    }

protected:
    void keyPressEvent(QKeyEvent* event) override
    {
        const Qt::KeyboardModifiers modifiers { event->modifiers() };

        if (event->key() == Qt::Key_Semicolon) {
            if (modifiers == Qt::ControlModifier) {
                insertPlainText(QDate::currentDate().toString(kDateFST));
                return;
            }
            if (modifiers == (Qt::ControlModifier | Qt::ShiftModifier)) {
                insertPlainText(QDateTime::currentDateTime().toString(kDateTimeFST));
                return;
            }
        }

        QPlainTextEdit::keyPressEvent(event);
    }
};

#endif // PLAINTEXTEDIT_H
