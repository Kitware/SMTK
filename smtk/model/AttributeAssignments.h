//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_AttributeAssignments_h
#define smtk_model_AttributeAssignments_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/common/UUID.h"

#include <map>
#include <set>

namespace smtk
{
namespace model
{

/**\brief Store a list of attributes assigned to solid model entities.
  *
  */
class SMTKCORE_EXPORT AttributeAssignments
{
public:
  bool associateAttribute(const smtk::common::UUID& attribId);
  bool disassociateAttribute(const smtk::common::UUID& attribId);
  bool isAssociated(const smtk::common::UUID& attribId) const;

  smtk::common::UUIDs& attributeIds() { return m_attributes; }
  const smtk::common::UUIDs& attributeIds() const { return m_attributes; }

protected:
  smtk::common::UUIDs m_attributes; // IDs of attributes assigned to an entity.
};

/// Each Manager entity's UUID is mapped to a set of assigned attribute IDs.
typedef std::map<smtk::common::UUID, AttributeAssignments> UUIDsToAttributeAssignments;
/// An iterator referencing a (UUID,AttributeAssignments)-tuple.
typedef std::map<smtk::common::UUID, AttributeAssignments>::iterator UUIDWithAttributeAssignments;

} // namespace model
} // namespace smtk

#endif // smtk_model_AttributeAssignments_h
