#ifndef TAGUTILS_H
#define TAGUTILS_H

#include <QColor>
#include <QString>
#include <QUuid>

#include "tag/tagrow.h"

struct SearchQuery {
    QString text {}; // Normal search text (without [tag])
    QSet<QString> tags {}; // Tag names or tag IDs (depending on your mapping)
};

namespace utils {

SearchQuery ParseSearchQuery(const QString& input, const QHash<QUuid, TagRow*>& tag_hash);
QColor GetContrastColor(const QColor& bg_color);
QIcon CreateTagIcon(const TagRow* tag, bool checked);
QPixmap CreateTagPixmap(const TagRow* tag);

}

#endif // TAGUTILS_H
