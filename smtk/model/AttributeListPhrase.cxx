#include "smtk/model/AttributeListPhrase.h"
//#include "smtk/model/AttributeValuePhrase.h"

#include "smtk/model/FloatData.h"
#include "smtk/model/StringData.h"
#include "smtk/model/IntegerData.h"

namespace smtk {
  namespace model {

AttributeListPhrase::AttributeListPhrase()
{
}

/**\brief Populate an attribute-list descriptive phrase with
  * a \a subset of attributes from the given entity.
  *
  * Only attribute IDs in \a subset will be displayed.
  * Note that \a subset must not contain any attribute IDs not present in entity.attributes().
  */
AttributeListPhrase::Ptr AttributeListPhrase::setup(
  const Cursor& entity, const AttributeSet& subset, DescriptivePhrasePtr parnt)
{
  this->m_entity = entity;
  this->m_attributes = subset;
  this->DescriptivePhrase::setup(ATTRIBUTE_LIST, parnt);
  return shared_from_this();
}

std::string AttributeListPhrase::title()
{
  std::ostringstream message;
  DescriptivePhrases::size_type sz =
    this->m_attributes.empty() ? this->m_entity.attributes().attributes().size() : this->m_attributes.size();
  message << sz << " " << (sz == 1 ? "attribute" : "attributes");
  return message.str();
}

std::string AttributeListPhrase::subtitle()
{
  return std::string();
}

smtk::util::UUID AttributeListPhrase::relatedEntityId() const
{
  return this->m_entity.entity();
}

Cursor AttributeListPhrase::relatedEntity() const
{
  return this->m_entity;
}

bool AttributeListPhrase::buildSubphrasesInternal()
{
  return true;
}

  } // model namespace
} // smtk namespace
