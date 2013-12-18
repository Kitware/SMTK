#ifndef __smtk_model_AttributeAssignments_h
#define __smtk_model_AttributeAssignments_h

#include "smtk/util/UUID.h"

#include "sparsehash/sparse_hash_map"

#include <set>

namespace smtk {
  namespace model {

/**\brief Store a list of attributes assigned to solid model entities.
  *
  */
class SMTKCORE_EXPORT AttributeAssignments
{
public:
  typedef int AttributeId;
  typedef std::set<AttributeId> AttributeSet;

  bool attachAttribute(int attribId);
  bool detachAttribute(int attribId);
  bool isAssociated(int attribId) const;

  AttributeSet& attributes() { return this->m_attributes; }
  const AttributeSet& attributes() const { return this->m_attributes; }

protected:
  AttributeSet m_attributes; // IDs of attributes assigned to an entity.
};

/// Each Storage entity's UUID is mapped to a set of assigned attribute IDs.
typedef google::sparse_hash_map<smtk::util::UUID,AttributeAssignments> UUIDsToAttributeAssignments;
/// An iterator referencing a (UUID,AttributeAssignments)-tuple.
typedef google::sparse_hash_map<smtk::util::UUID,AttributeAssignments>::iterator UUIDWithAttributeAssignments;

  } // model namespace
} // smtk namespace

#endif // __smtk_model_AttributeAssignments_h
