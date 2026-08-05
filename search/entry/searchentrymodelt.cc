#include "searchentrymodelt.h"

namespace search {

EntryModelT::EntryModelT(CSectionInfo& info, const QHash<QUuid, tag::Row*>& tag_hash, QObject* parent)
    : EntryModel { info, tag_hash, parent }
{
}
}