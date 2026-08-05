#ifndef TAGUTILS_H
#define TAGUTILS_H

#include <QColor>
#include <QString>
#include <QUuid>

#include "tag/tagrow.h"

namespace tag {

struct SearchQuery final {
    QString text {}; // Normal search text (without [tag])
    QSet<QString> tags {}; // Tag names or tag IDs (depending on your mapping)
};

SearchQuery ParseSearchQuery(const QString& input, const QHash<QUuid, Row*>& tag_hash);
QColor GetContrastColor(const QColor& bg_color);
QIcon CreateIcon(const Row* tag, bool checked);
QPixmap CreatePixmap(const Row* tag);

}

#endif // TAGUTILS_H
