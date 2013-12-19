#ifndef __smtk_model_UseEntity_h
#define __smtk_model_UseEntity_h

#include "smtk/model/Cursor.h"
#include "smtk/model/CursorArrangementOps.h" // For shellsAs<T>().

namespace smtk {
  namespace model {

class CellEntity;
class UseEntity;
typedef std::vector<UseEntity> UseEntities;

/**\brief A cursor subclass that provides methods specific to entity-use records.
  *
  * An entity-use record provides a way to reference how a cell is
  * employed (**used**) to bound a higher-dimensional cell.
  * Entity-use records reference a particular **sense** in which their
  * corresponding cell is used (e.g., an edge may used in its forward or
  * backward sense). Entity-use records are associated with a particular
  * higher-dimensional half-space that their corresponding cell defines
  * (or helps define).
  *
  * An edge's forward or backward senses may each be used by any number
  * of different face loops.
  *
  * A face has 0, 1, or 2 use records, depending on whether it appears
  * in the shells of 0, 1, or 2 volumes.
  *
  * A vertex may have any number of use records; one should exist for each
  * volume, free face, or free edge attached to the vertex.
  */
class SMTKCORE_EXPORT UseEntity : public Cursor
{
public:
  SMTK_CURSOR_CLASS(UseEntity,Cursor,isUseEntity);

  CellEntity cell() const;
  template<typename T> T shellsAs() const;
};

template<typename T> T UseEntity::shellsAs() const
{
  T container;
  CursorArrangementOps::appendAllRelations(*this, HAS_SHELL, container);
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_UseEntity_h
