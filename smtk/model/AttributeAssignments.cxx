#include "smtk/model/AttributeAssignments.h"

using smtk::attribute::AttributeId;

namespace smtk {
 namespace model {

bool AttributeAssignments::attachAttribute(AttributeId attribId)
{
  return this->m_attributes.insert(attribId).second;
}

bool AttributeAssignments::detachAttribute(AttributeId attribId)
{
  return this->m_attributes.erase(attribId) > 0;
}

bool AttributeAssignments::isAssociated(AttributeId attribId) const
{
  return this->m_attributes.find(attribId) == this->m_attributes.end() ?
    false : true;
}

  } //namespace model
} // namespace smtk
