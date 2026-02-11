#include "tagutils.h"

#include <QtGui/qicon.h>
#include <QtWidgets/qapplication.h>

#include <QPainter>
#include <QPixmap>

SearchQuery Utils::ParseSearchQuery(const QString& input, const QHash<QUuid, Tag*>& tag_hash)
{
    SearchQuery query {};

    static const QRegularExpression kTagRegex { R"(\[([^\]]+)\])" };

    QRegularExpressionMatchIterator it = kTagRegex.globalMatch(input);

    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const QString tag_name = match.captured(1).trimmed();

        if (tag_name.isEmpty())
            continue;

        // Resolve tag name -> tag id
        for (auto it = tag_hash.cbegin(); it != tag_hash.cend(); ++it) {
            const Tag* tag = it.value();
            if (!tag)
                continue;

            if (tag->name.compare(tag_name, Qt::CaseInsensitive) == 0) {
                query.tags.insert(it.key().toString(QUuid::WithoutBraces));
                break;
            }
        }
    }

    // Remove all [xxx] from text
    QString text = input;
    text.remove(kTagRegex);
    query.text = text.trimmed();

    return query;
}

QIcon Utils::CreateTagIcon(const Tag* tag, bool checked)
{
    const qreal dpr { qApp->devicePixelRatio() };
    QPixmap pixmap(static_cast<int>(16 * dpr), static_cast<int>(16 * dpr));
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(QColor(tag->color)));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(1, 1, 15, 15, 3, 3);

    if (checked) {
        painter.setPen(GetContrastColor(tag->color));
        painter.setBrush(Qt::NoBrush);
        painter.drawLine(QPointF(4, 8), QPointF(7, 11));
        painter.drawLine(QPointF(7, 11), QPointF(12, 5));
    }

    painter.end();

    QIcon icon(pixmap);
    return icon;
}

QPixmap Utils::CreateTagPixmap(const Tag* tag)
{
    const qreal dpr { qApp->devicePixelRatio() };

    // Font in logical pixels
    QFont font { qApp->font() };
    font.setPointSize(font.pointSize() - 2);
    QFontMetrics fm { font };

    const int text_width { fm.horizontalAdvance(tag->name) };
    const int tag_width { text_width + 2 * kTagHPadding };
    const int tag_height { fm.height() + 2 * kTagVPadding };

    // Create physical pixmap (actual pixels = logical pixels * dpr)
    QPixmap pixmap { static_cast<int>(tag_width * dpr), static_cast<int>(tag_height * dpr) };
    pixmap.setDevicePixelRatio(dpr); // After this, use logical coordinates for drawing
    pixmap.fill(Qt::transparent);

    QPainter painter { &pixmap };
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(font);

    // Draw using logical coordinates (no manual scaling needed)
    painter.setPen(Qt::NoPen);
    painter.setBrush(tag->color);

    // Draw background with integer coordinates to avoid blurriness
    const QRect background_rect { 0, 0, tag_width, tag_height };
    painter.drawRoundedRect(background_rect, kTagRadius, kTagRadius);

    // Draw text
    painter.setPen(Utils::GetContrastColor(tag->color));
    painter.drawText(background_rect, Qt::AlignCenter, tag->name);

    return pixmap;
}

QColor Utils::GetContrastColor(const QColor& bg_color)
{
    const int brightness { (bg_color.red() * 299 + bg_color.green() * 587 + bg_color.blue() * 114) / 1000 };
    return brightness > 128 ? Qt::black : Qt::white;
}
