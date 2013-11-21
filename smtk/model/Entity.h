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

class SMTKCORE_EXPORT Entity
{
public:
  Entity();
  Entity(unsigned int entityFlags, int dimension);

  int dimension() const;
  unsigned int dimensionBits() const;
  unsigned int entityFlags() const;
  bool setEntityFlags(unsigned int flags);

  smtk::util::UUIDArray& relations();
  const smtk::util::UUIDArray& relations() const;

  Entity& appendRelation(const smtk::util::UUID& b);
  Entity& removeRelation(const smtk::util::UUID& b);

  static std::string flagSummary(unsigned int entityFlags);
  static std::string flagDescription(unsigned int entityFlags);

protected:
  unsigned int m_entityFlags;
  smtk::util::UUIDArray m_relations;
private:
};

typedef std::pair<smtk::util::UUID,Entity> UUIDEntityPair;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Entity_h
