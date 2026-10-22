#include "searchentrymodeli.h"

namespace search {
EntryModelI::EntryModelI(Section section, const QStringList& header, const QHash<QUuid, tag::Row>& tag_hash, QObject* parent)
    : EntryModel { section, header, tag_hash, parent }
{
}
}
