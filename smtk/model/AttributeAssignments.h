#ifndef __smtk_model_AttributeAssignments_h
#define __smtk_model_AttributeAssignments_h

#include "smtk/util/UUID.h"

#include "smtk/options.h" // for SMTK_HASH_STORAGE
#ifdef SMTK_HASH_STORAGE
#  include "sparsehash/sparse_hash_map"
#else
#  include <map>
#endif
#include <set>

namespace smtk {
  namespace model {

/// A type for attribute identifiers.
typedef unsigned long AttributeId;

/// A set of attribute identifiers.
typedef std::set<AttributeId> AttributeSet;

/**\brief Store a list of attributes assigned to solid model entities.
  *
  */
class SMTKCORE_EXPORT AttributeAssignments
{
public:
  bool attachAttribute(int attribId);
  bool detachAttribute(int attribId);
  bool isAssociated(int attribId) const;

  AttributeSet& attributes() { return this->m_attributes; }
  const AttributeSet& attributes() const { return this->m_attributes; }

protected:
  AttributeSet m_attributes; // IDs of attributes assigned to an entity.
};

#ifdef SMTK_HASH_STORAGE
/// Each Storage entity's UUID is mapped to a set of assigned attribute IDs.
typedef google::sparse_hash_map<smtk::util::UUID,AttributeAssignments> UUIDsToAttributeAssignments;
/// An iterator referencing a (UUID,AttributeAssignments)-tuple.
typedef google::sparse_hash_map<smtk::util::UUID,AttributeAssignments>::iterator UUIDWithAttributeAssignments;
#else
/// Each Storage entity's UUID is mapped to a set of assigned attribute IDs.
typedef std::map<smtk::util::UUID,AttributeAssignments> UUIDsToAttributeAssignments;
/// An iterator referencing a (UUID,AttributeAssignments)-tuple.
typedef std::map<smtk::util::UUID,AttributeAssignments>::iterator UUIDWithAttributeAssignments;
#endif

  } // model namespace
} // smtk namespace

#endif // __smtk_model_AttributeAssignments_h
