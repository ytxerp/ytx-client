#include "searchentrymodeli.h"

SearchEntryModelI::SearchEntryModelI(CSectionInfo& info, const QHash<QUuid, Tag*>& tag_hash, QObject* parent)
    : SearchEntryModel { info, tag_hash, parent }
{
}
