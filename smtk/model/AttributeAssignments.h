//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_AttributeAssignments_h
#define __smtk_model_AttributeAssignments_h

#include "smtk/options.h" // for SMTK_HASH_STORAGE
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/UUID.h"

#ifdef SMTK_HASH_STORAGE
#  if defined(_MSC_VER) // Visual studio
#    pragma warning (push)
#    pragma warning (disable : 4996)  // Overeager "unsafe" parameter check
#  endif
#  include "sparsehash/sparse_hash_map"
#  if defined(_MSC_VER) // Visual studio
#    pragma warning (pop)
#  endif
#else
#  include <map>
#endif
#include <set>

namespace smtk {
  namespace model {

/// A set of attribute identifiers.
typedef std::set<smtk::common::UUID> AttributeSet;

/**\brief Store a list of attributes assigned to solid model entities.
  *
  */
class SMTKCORE_EXPORT AttributeAssignments
{
public:
  bool attachAttribute(const smtk::common::UUID &attribId);
  bool detachAttribute(const smtk::common::UUID &attribId);
  bool isAssociated(const smtk::common::UUID &attribId) const;

  AttributeSet& attributes() { return this->m_attributes; }
  const AttributeSet& attributes() const { return this->m_attributes; }

protected:
  AttributeSet m_attributes; // IDs of attributes assigned to an entity.
};

#ifdef SMTK_HASH_STORAGE
/// Each Manager entity's UUID is mapped to a set of assigned attribute IDs.
typedef google::sparse_hash_map<smtk::common::UUID,AttributeAssignments> UUIDsToAttributeAssignments;
/// An iterator referencing a (UUID,AttributeAssignments)-tuple.
typedef google::sparse_hash_map<smtk::common::UUID,AttributeAssignments>::iterator UUIDWithAttributeAssignments;
#else
/// Each Manager entity's UUID is mapped to a set of assigned attribute IDs.
typedef std::map<smtk::common::UUID,AttributeAssignments> UUIDsToAttributeAssignments;
/// An iterator referencing a (UUID,AttributeAssignments)-tuple.
typedef std::map<smtk::common::UUID,AttributeAssignments>::iterator UUIDWithAttributeAssignments;
#endif

  } // model namespace
} // smtk namespace

#endif // __smtk_model_AttributeAssignments_h
