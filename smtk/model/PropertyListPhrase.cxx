#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/PropertyValuePhrase.h"

#include "smtk/model/FloatData.h"
#include "smtk/model/StringData.h"
#include "smtk/model/IntegerData.h"

namespace smtk {
  namespace model {

PropertyListPhrase::PropertyListPhrase()
  : m_propertyType(INVALID_PROPERTY)
{
}

PropertyListPhrase::Ptr PropertyListPhrase::setup(
  const Cursor& entity, PropertyType ptype, DescriptivePhrasePtr parnt)
{
  this->m_entity = entity;
  this->m_propertyType = ptype;
  this->DescriptivePhrase::setup(
    PropertyListPhrase::propertyToPhraseType(ptype),
    parnt);
  return shared_from_this();
}

std::string PropertyListPhrase::title()
{
  std::ostringstream message;
  DescriptivePhrases::size_type sz = this->subphrases().size();
  message << sz << " " <<
    (this->m_propertyType == FLOAT_PROPERTY ? "floating-point" :
     (this->m_propertyType == STRING_PROPERTY ? "string" :
      (this->m_propertyType == INTEGER_PROPERTY ? "integer" : "invalid")))
     << " " << (sz == 1 ? "property" : "properties");
  return message.str();
}

std::string PropertyListPhrase::subtitle()
{
  return std::string();
}

smtk::util::UUID PropertyListPhrase::relatedEntityId() const
{
  return this->m_entity.entity();
}

Cursor PropertyListPhrase::relatedEntity() const
{
  return this->m_entity;
}

std::string PropertyListPhrase::relatedPropertyName() const
{
  return std::string(); // lists do not have a single name; their subphrases do.
}

PropertyType PropertyListPhrase::relatedPropertyType() const
{
  return this->m_propertyType;
}

DescriptivePhraseType PropertyListPhrase::propertyToPhraseType(PropertyType p)
{
  switch (p)
    {
  case FLOAT_PROPERTY:
    return FLOAT_PROPERTY_LIST;
  case STRING_PROPERTY:
    return STRING_PROPERTY_LIST;
  case INTEGER_PROPERTY:
    return INTEGER_PROPERTY_LIST;
  case INVALID_PROPERTY:
  default:
    break;
    }
  return INVALID_DESCRIPTION;
}

bool PropertyListPhrase::buildSubphrasesInternal()
{
  switch (this->m_propertyType)
    {
  case FLOAT_PROPERTY:
    if (this->m_entity.hasFloatProperties())
      {
      for (
        PropertyNameWithConstFloats it = this->m_entity.floatProperties().begin();
        it != this->m_entity.floatProperties().end();
        ++it)
        {
        this->m_subphrases.push_back(
          PropertyValuePhrase::create()->setup(it->first, shared_from_this()));
        }
      }
    break;
  case STRING_PROPERTY:
    if (this->m_entity.hasStringProperties())
      {
      for (
        PropertyNameWithConstStrings it = this->m_entity.stringProperties().begin();
        it != this->m_entity.stringProperties().end();
        ++it)
        {
        if (it->first != "name")
          {
          this->m_subphrases.push_back(
            PropertyValuePhrase::create()->setup(it->first, shared_from_this()));
          }
        }
      }
    break;
  case INTEGER_PROPERTY:
    if (this->m_entity.hasIntegerProperties())
      {
      for (
        PropertyNameWithConstIntegers it = this->m_entity.integerProperties().begin();
        it != this->m_entity.integerProperties().end();
        ++it)
        {
        this->m_subphrases.push_back(
          PropertyValuePhrase::create()->setup(it->first, shared_from_this()));
        }
      }
    break;
  case INVALID_PROPERTY:
  default:
    break;
    }
  return true;
}

  } // model namespace
} // smtk namespace
