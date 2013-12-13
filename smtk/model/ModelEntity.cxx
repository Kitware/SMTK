#include "smtk/model/ModelEntity.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Arrangement.h"

namespace smtk {
  namespace model {

CellEntities ModelEntity::cells() const
{
  CellEntities result;
  if (this->isValid())
    {
    UUIDsToArrangements& all(this->m_storage->arrangements());
    UUIDWithArrangementDictionary cellEntry = all.find(this->m_entity);
    if (cellEntry != all.end())
      {
      ArrangementKindWithArrangements useIt = cellEntry->second.find(INCLUDES);
      if (useIt != cellEntry->second.end())
        {
        Entity* entRec = this->m_storage->findEntity(this->m_entity);
        if (entRec)
          {
          smtk::util::UUIDArray const& relations(entRec->relations());
          for (Arrangements::iterator arrIt = useIt->second.begin(); arrIt != useIt->second.end(); ++arrIt)
            {
            for (std::vector<int>::iterator dit = arrIt->details().begin(); dit != arrIt->details().end(); ++dit)
              {
              result.push_back(CellEntity(this->m_storage, relations[*dit]));
              }
            }
          }
        }
      }
    }
  return result;
}

  } // namespace model
} // namespace smtk
