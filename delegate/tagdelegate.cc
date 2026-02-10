#include "tagdelegate.h"

#include <QPainter>

#include "utils/mainwindowutils.h"

TagDelegate::TagDelegate(const QHash<QUuid, Tag*>& tag_hash, QObject* parent)
    : StyledItemDelegate { parent }
    , tag_hash_ { tag_hash }
{
}

void TagDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QStringList tag_ids { index.data().toStringList() };
    if (tag_ids.isEmpty()) {
        return PaintEmpty(painter, option, index);
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QFont font { option.font };
    font.setPointSize(font.pointSize() - 1);
    const QFontMetrics fm { font };
    painter->setFont(font);

    int x { option.rect.left() + kMargin };
    const int tag_height { option.rect.height() - 2 * kVPadding }; // Shrink inside the cell
    const int y { option.rect.top() + kVPadding }; // Top padding

    for (const QString& id_str : tag_ids) {
        const QUuid tag_id { QUuid::fromString(id_str) };
        if (tag_id.isNull())
            continue;

        const auto it { tag_hash_.find(tag_id) };
        if (it == tag_hash_.end() || !it.value())
            continue;

        const Tag* tag { it.value() };
        const int text_width { fm.horizontalAdvance(tag->name) };
        const int tag_width { text_width + 2 * kHPadding };

        const QRect tag_rect { x, y, tag_width, tag_height };
        painter->setPen(Qt::NoPen);
        painter->setBrush(tag->color);
        painter->drawRoundedRect(tag_rect, kRadius, kRadius);

        painter->setPen(Utils::GetContrastColor(tag->color));
        painter->drawText(tag_rect, Qt::AlignCenter, tag->name);

        x += tag_width + kTagSpacing;
    }

    painter->restore();
}

QSize TagDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QStringList tag_ids { index.data().toStringList() };

    if (tag_ids.isEmpty()) {
        return {};
    }

    QFont font { option.font };
    font.setPointSize(font.pointSize() - 1);
    const QFontMetrics fm { font };

    int total_width { kMargin * 2 }; // Left and right margin

    for (const QString& id_str : tag_ids) {
        const QUuid tag_id { QUuid::fromString(id_str) };
        const auto it { tag_hash_.find(tag_id) };

        if (it != tag_hash_.end() && it.value()) {
            const int text_width { fm.horizontalAdvance(it.value()->name) };
            total_width += text_width + 2 * kHPadding + kTagSpacing; // Add tag width + horizontal padding + spacing
        }
    }

    total_width -= kTagSpacing; // Subtract the extra spacing after the last tag

    const int height { fm.height() + 2 * kVPadding + 2 * kMargin }; // Tag height + vertical padding + top/bottom margin

    return QSize(total_width, height);
}
