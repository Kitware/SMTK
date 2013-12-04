#include "smtk/model/ShellEntity.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Storage.h"
#include "smtk/model/UseEntity.h"

namespace smtk {
  namespace model {

CellEntity ShellEntity::parentCell() const
{
  if (this->isValid())
    {
    Arrangements* arr = NULL;
    if (
      (arr = this->m_storage->hasArrangementsOfKindForEntity(this->m_entity, HAS_CELL)) &&
      !arr->empty())
      {
      Entity* entRec = this->m_storage->findEntity(this->m_entity);
      if (entRec)
        {
        smtk::util::UUIDArray const& relations(entRec->relations());
        for (Arrangements::iterator arrIt = arr->begin(); arrIt != arr->end(); ++arrIt)
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
  return CellEntity();
}

/**\brief Return the uses (cells with an orientation, or sense) composing this shell.
  *
  * When the shell is 1-dimensional, these uses will be ordered curve segments
  * that match head-to-tail and describe a closed loop.
  *
  * TODO: Decide how to handle multiple closed loops: should an "empty" shell (that
  * returns uses().empty()) be created with multiple containedShells()? Or should
  * shells be allowed to have siblings?
  */
UseEntities ShellEntity::uses() const
{
  return UseEntities();
}

  } // namespace model
} // namespace smtk
