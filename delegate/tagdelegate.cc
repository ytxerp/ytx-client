#include "tagdelegate.h"

#include <QtWidgets/qapplication.h>

#include <QPainter>

TagDelegate::TagDelegate(const QHash<QUuid, Tag*>& tag_hash, const QHash<QUuid, TagIcons>& tag_icons_hash, QObject* parent)
    : StyledItemDelegate { parent }
    , tag_hash_ { tag_hash }
    , tag_icons_hash_ { tag_icons_hash }
{
}

void TagDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QStringList tag_ids { index.data().toStringList() };
    if (tag_ids.isEmpty()) {
        return PaintEmpty(painter, option, index);
    }

    QStyle* style { option.widget ? option.widget->style() : QApplication::style() };
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    int x { option.rect.left() + kTagMargin };

    for (const QString& id_str : tag_ids) {
        const QUuid tag_id { QUuid::fromString(id_str) };
        if (tag_id.isNull())
            continue;

        const auto it { tag_icons_hash_.constFind(tag_id) };
        if (it == tag_icons_hash_.constEnd()) {
            continue;
        }

        const QPixmap& pixmap { it->pixmap };
        if (pixmap.isNull()) {
            continue;
        }

        // Logical size (important!)
        const int logical_width { qRound(pixmap.width() / pixmap.devicePixelRatio()) };
        const int logical_height { qRound(pixmap.height() / pixmap.devicePixelRatio()) };

        // Vertically center inside the item rect
        const int y { option.rect.top() + (option.rect.height() - logical_height) / 2 };

        painter->drawPixmap(x, y, pixmap);

        x += logical_width + kTagSpacing;
    }

    painter->restore();
}

QSize TagDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QStringList tag_ids { index.data().toStringList() };
    if (tag_ids.isEmpty()) {
        return QSize(0, option.rect.height());
    }

    int total_width { kTagMargin * 2 };
    bool has_valid_tag { false };

    for (const QString& id_str : tag_ids) {
        const QUuid tag_id { QUuid::fromString(id_str) };
        const auto it { tag_icons_hash_.constFind(tag_id) };

        if (it == tag_icons_hash_.constEnd()) {
            continue;
        }

        const QPixmap& pixmap { it->pixmap };
        if (pixmap.isNull()) {
            continue;
        }

        const int logical_width { qRound(pixmap.width() / pixmap.devicePixelRatio()) };

        total_width += logical_width + kTagSpacing;
        has_valid_tag = true;
    }

    if (has_valid_tag) {
        total_width -= kTagSpacing;
    }

    return QSize(total_width, option.rect.height());
}
