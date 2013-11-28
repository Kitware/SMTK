#include "smtk/model/CellEntity.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/Storage.h"

namespace smtk {
  namespace model {

CellEntity::CellEntity()
{
}

CellEntity::CellEntity(const Cursor& other)
  : Cursor(other)
{
}

CellEntity::CellEntity(StoragePtr storage, const smtk::util::UUID& uid)
  : Cursor(storage, uid)
{
}

UseEntities CellEntity::uses() const
{
  UseEntities result;
  if (this->isValid())
    {
    UUIDsToArrangements& all(this->m_storage->arrangements());
    UUIDWithArrangementDictionary cellEntry = all.find(this->m_entity);
    if (cellEntry != all.end())
      {
      ArrangementKindWithArrangements useIt = cellEntry->second.find(HAS_USE);
      if (useIt != cellEntry->second.end())
        {
        Entity* entRec = this->m_storage->findEntity(this->m_entity);
        if (entRec)
          {
          smtk::util::UUIDArray const& relations(entRec->relations());
          for (Arrangements::iterator arrIt = useIt->second.begin(); arrIt != useIt->second.end(); ++arrIt)
            {
            for (std::vector<int>::iterator it = arrIt->details().begin(); it != arrIt->details().end(); ++it)
              {
              result.insert(UseEntity(this->m_storage, relations[*it]));
              //++it; // Err. Don't Skip every other entry, as it is an offset into the use-entity's .
              // Because we keep use records separate from cells now.
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
