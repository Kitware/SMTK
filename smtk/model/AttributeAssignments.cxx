//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/AttributeAssignments.h"

namespace smtk {
 namespace model {

   bool AttributeAssignments::attachAttribute(const smtk::common::UUID &attribId)
{
  return this->m_attributes.insert(attribId).second;
}

bool AttributeAssignments::detachAttribute(const smtk::common::UUID &attribId)
{
  return this->m_attributes.erase(attribId) > 0;
}

bool AttributeAssignments::isAssociated(const smtk::common::UUID &attribId) const
{
  return this->m_attributes.find(attribId) == this->m_attributes.end() ?
    false : true;
}

  } //namespace model
} // namespace smtk
