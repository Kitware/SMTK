#include "smtk/model/AttributeAssignments.h"

namespace smtk {
 namespace model {

bool AttributeAssignments::attachAttribute(std::size_t attribId)
{
  return this->m_attributes.insert(attribId).second;
}

bool AttributeAssignments::detachAttribute(std::size_t attribId)
{
  return this->m_attributes.erase(attribId) > 0;
}

bool AttributeAssignments::isAssociated(std::size_t attribId) const
{
  return this->m_attributes.find(attribId) == this->m_attributes.end() ?
    false : true;
}

  } //namespace model
} // namespace smtk
