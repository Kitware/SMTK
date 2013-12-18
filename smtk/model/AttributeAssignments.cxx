#include "smtk/model/AttributeAssignments.h"

namespace smtk {
 namespace model {

bool AttributeAssignments::attachAttribute(int attribId)
{
  return this->m_attributes.insert(attribId).second;
}

bool AttributeAssignments::detachAttribute(int attribId)
{
  return this->m_attributes.erase(attribId) > 0;
}

bool AttributeAssignments::isAssociated(int attribId) const
{
  return this->m_attributes.find(attribId) == this->m_attributes.end() ?
    false : true;
}

  } //namespace model
} // namespace smtk
