#include "smtk/model/UseEntity.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Arrangement.h"

namespace smtk {
  namespace model {

CellEntity UseEntity::cell() const
{
  if (this->isValid())
    {
    UUIDsToArrangements& all(this->m_storage->arrangements());
    UUIDWithArrangementDictionary cellEntry = all.find(this->m_entity);
    if (cellEntry != all.end())
      {
      ArrangementKindWithArrangements useIt = cellEntry->second.find(HAS_CELL);
      if (useIt != cellEntry->second.end())
        {
        Entity* entRec = this->m_storage->findEntity(this->m_entity);
        if (entRec)
          {
          smtk::util::UUIDArray const& relations(entRec->relations());
          for (Arrangements::iterator arrIt = useIt->second.begin(); arrIt != useIt->second.end(); ++arrIt)
            {
            // Return the first cell referenced in the first non-empty HAS_CELL arrangement:
            if (!arrIt->details().empty())
              {
              return CellEntity(this->m_storage, relations[arrIt->details().front()]);
              }
            }
          }
        }
      }
    }
  return CellEntity();
}

  } // namespace model
} // namespace smtk
