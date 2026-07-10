#include "searchentrymodeli.h"

namespace search {
EntryModelI::EntryModelI(CSectionInfo& info, const QHash<QUuid, Tag*>& tag_hash, QObject* parent)
    : EntryModel { info, tag_hash, parent }
{
}
}
