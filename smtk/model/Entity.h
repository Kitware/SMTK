#ifndef __smtk_model_Entity_h
#define __smtk_model_Entity_h

#include "smtk/SMTKCoreExports.h" // for SMTKCORE_EXPORT macro
#include "smtk/model/EntityTypeBits.h" // for entityFlags values

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "smtk/util/UUID.h"

namespace smtk {
  namespace model {

typedef std::set<smtk::util::UUID> UUIDs;
typedef std::vector<smtk::util::UUID> UUIDArray;
typedef std::vector<UUIDArray> UUIDArrays;

class SMTKCORE_EXPORT Entity
{
public:
  Entity();
  Entity(unsigned int entityFlags, int dimension);

  int dimension() const;
  unsigned int dimensionBits() const;
  unsigned int entityFlags() const;

  UUIDArray& relations();
  const UUIDArray& relations() const;

  Entity& appendRelation(const smtk::util::UUID& b);
  Entity& removeRelation(const smtk::util::UUID& b);

protected:
  unsigned int m_entityFlags;
  UUIDArray m_relations;
private:
};

typedef std::pair<smtk::util::UUID,Entity> UUIDEntityPair;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Entity_h
