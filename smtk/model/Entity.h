#ifndef __smtk_model_Entity_h
#define __smtk_model_Entity_h

#include "smtk/SMTKCoreExports.h" // for SMTKCORE_EXPORT macro
#include "smtk/model/EntityTypeBits.h" // for entityFlags values
#include "smtk/model/IntegerData.h" // for IntegerList
#include "smtk/util/SystemConfig.h"

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "smtk/util/UUID.h"

namespace smtk {
  namespace model {

/**\brief A solid model entity, defined by a type and relations to other entities.
  *
  * A solid model is an smtk::model::Storage instance that maps UUIDs to
  * to records of various types. Every entity (topological cell, geometric,
  * group, submodel, or scene-graph instance) must have an Entity record
  * describing the type of the entity and relating it to other entities.
  * Other records, such as string, integer, or vector properties are optional.
  *
  * The entity type is stored as a bit vector named \a entityFlags().
  * This also encodes the parametric dimension (or dimensions) associated
  * with the entity.
  */
class SMTKCORE_EXPORT Entity
{
public:
  Entity();
  Entity(BitFlags entityFlags, int dimension);

  int dimension() const;
  BitFlags dimensionBits() const;
  BitFlags entityFlags() const;
  bool setEntityFlags(BitFlags flags);

  smtk::util::UUIDArray& relations();
  const smtk::util::UUIDArray& relations() const;

  Entity& appendRelation(const smtk::util::UUID& b);
  Entity& removeRelation(const smtk::util::UUID& b);

  int findOrAppendRelation(const smtk::util::UUID& r);

  std::string flagSummary(int form = 0) const
    { return Entity::flagSummary(this->entityFlags(), form); }
  std::string flagDescription(int form = 0) const
    { return Entity::flagDescription(this->entityFlags(), form); }

  static std::string flagDimensionList(BitFlags entityFlags, bool& plural);
  static std::string flagSummaryHelper(BitFlags entityFlags, int form = 0);
  static std::string flagSummary(BitFlags entityFlags, int form = 0);
  static std::string flagDescription(BitFlags entityFlags, int form = 0);
  static std::string defaultNameFromCounters(BitFlags entityFlags, IntegerList& counters);

protected:
  BitFlags m_entityFlags;
  smtk::util::UUIDArray m_relations;
private:
};

typedef std::pair<smtk::util::UUID,Entity> UUIDEntityPair;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Entity_h
