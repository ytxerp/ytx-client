#include "tagdelegate.h"

#include <QtWidgets/qapplication.h>

#include <QPainter>

TagDelegate::TagDelegate(const QHash<QUuid, Tag*>& tag_hash, const QHash<QUuid, QPixmap>& tag_pixmap, QObject* parent)
    : StyledItemDelegate { parent }
    , tag_hash_ { tag_hash }
    , tag_pixmap_ { tag_pixmap }
{
}

void TagDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QStringList tag_ids { index.data().toStringList() };
    if (tag_ids.isEmpty()) {
        return PaintEmpty(painter, option, index);
    }

    static QStyle* style { QApplication::style() };
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    int x { option.rect.left() + kTagMargin };

    for (const QString& id_str : tag_ids) {
        const QUuid tag_id { QUuid::fromString(id_str) };
        if (tag_id.isNull())
            continue;

        const auto it { tag_pixmap_.find(tag_id) };
        if (it == tag_pixmap_.end() || it->isNull())
            continue;

        const QPixmap& pixmap { *it };

        // Logical size (important!)
        const int logical_width { static_cast<int>(pixmap.width() / pixmap.devicePixelRatio()) };
        const int logical_height { static_cast<int>(pixmap.height() / pixmap.devicePixelRatio()) };

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
        return {};
    }

    int total_width { kTagMargin * 2 };

    for (const QString& id_str : tag_ids) {
        const QUuid tag_id { QUuid::fromString(id_str) };
        const auto pixmap_it { tag_pixmap_.find(tag_id) };

        if (pixmap_it != tag_pixmap_.end() && !pixmap_it->isNull()) {
            const int logical_width { static_cast<int>(pixmap_it->width() / pixmap_it->devicePixelRatio()) };
            total_width += logical_width + kTagSpacing;
        }
    }

    if (total_width > kTagMargin * 2) {
        total_width -= kTagSpacing;
    }

    return QSize(total_width, option.rect.height());
}
