#include "smtk/model/CellEntity.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/Storage.h"

namespace smtk {
  namespace model {

/**\brief Report all of the "use" records associated with the cell.
  *
  * The uses can be used to discover higher-dimensional cells that
  * this cell borders.
  * Each sense of a cell has its own use.
  */
UseEntities CellEntity::uses() const
{
  UseEntities result;
  Entity* entRec;
  if (this->isValid(&entRec))
    {
    Arrangements* arr = NULL;
    if (
      (arr = this->m_storage->hasArrangementsOfKindForEntity(this->m_entity, HAS_CELL)) &&
      !arr->empty())
      {
      smtk::util::UUIDArray const& relations(entRec->relations());
      for (Arrangements::iterator arrIt = arr->begin(); arrIt != arr->end(); ++arrIt)
        {
        for (std::vector<int>::iterator it = arrIt->details().begin(); it != arrIt->details().end(); ++it)
          {
          result.push_back(UseEntity(this->m_storage, relations[*it]));
          //++it; // Err. Don't Skip every other entry, as it is an offset into the use-entity's .
          // Because we keep use records separate from cells now.
          }
        }
      }
    }
  return result;
}

  } // namespace model
} // namespace smtk
