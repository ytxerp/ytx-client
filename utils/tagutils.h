#ifndef TAGUTILS_H
#define TAGUTILS_H

#include <QColor>
#include <QString>
#include <QUuid>

#include "tag/tag.h"

struct SearchQuery {
    QString text {}; // Normal search text (without [tag])
    QSet<QString> tags {}; // Tag names or tag IDs (depending on your mapping)
};

namespace Utils {

SearchQuery ParseSearchQuery(const QString& input, const QHash<QUuid, Tag*>& tag_hash);
QColor GetContrastColor(const QColor& bg_color);
QIcon CreateTagIcon(const Tag* tag, bool checked);
QPixmap CreateTagPixmap(const Tag* tag);

}

#endif // TAGUTILS_H
