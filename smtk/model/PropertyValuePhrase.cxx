#include "smtk/model/PropertyValuePhrase.h"
#include "smtk/model/PropertyListPhrase.h"

#include "smtk/model/FloatData.h"
#include "smtk/model/StringData.h"
#include "smtk/model/IntegerData.h"

namespace smtk {
  namespace model {

PropertyValuePhrase::PropertyValuePhrase()
{
}

PropertyValuePhrase::Ptr PropertyValuePhrase::setup(const std::string& propName, PropertyListPhrase::Ptr parnt)
{
  this->DescriptivePhrase::setup(
    PropertyValuePhrase::propertyToPhraseType(
      parnt->relatedPropertyType()),
    parnt);
  this->m_propertyName = propName;
  this->m_plist = parnt;
  return static_pointer_cast<SelfType>(shared_from_this());
}

std::string PropertyValuePhrase::title()
{
  return this->m_propertyName;
}

std::string PropertyValuePhrase::subtitle()
{
  std::ostringstream message;
  if (this->m_plist)
    {
    Cursor ent = this->m_plist->relatedEntity();
    switch (this->m_plist->relatedPropertyType())
      {
    case FLOAT_PROPERTY:
      if (ent.hasFloatProperty(this->m_propertyName))
        {
        FloatList const& prop(ent.floatProperty(this->m_propertyName));
        bool first = true;
        for (
          FloatList::const_iterator it = prop.begin();
          it != prop.end();
          ++it)
          {
          if (!first)
            {
            message << ", ";
            }
          message << *it;
          first = false;
          }
        }
      break;
    case STRING_PROPERTY:
      if (ent.hasStringProperty(this->m_propertyName))
        {
        StringList const& prop(ent.stringProperty(this->m_propertyName));
        bool first = true;
        for (
          StringList::const_iterator it = prop.begin();
          it != prop.end();
          ++it)
          {
          if (!first)
            {
            message << ", ";
            }
          message << *it;
          first = false;
          }
        }
      break;
    case INTEGER_PROPERTY:
      if (ent.hasIntegerProperty(this->m_propertyName))
        {
        IntegerList const& prop(ent.integerProperty(this->m_propertyName));
        bool first = true;
        for (
          IntegerList::const_iterator it = prop.begin();
          it != prop.end();
          ++it)
          {
          if (!first)
            {
            message << ", ";
            }
          message << *it;
          first = false;
          }
        }
      break;
    case INVALID_PROPERTY:
      message << "Invalid property type";
      break;
      }
    }
  return message.str();
}

smtk::util::UUID PropertyValuePhrase::relatedEntityId() const
{
  return this->m_plist->relatedEntityId();
}

Cursor PropertyValuePhrase::relatedEntity() const
{
  return this->m_plist->relatedEntity();
}

std::string PropertyValuePhrase::relatedPropertyName() const
{
  return this->m_propertyName;
}

PropertyType PropertyValuePhrase::relatedPropertyType() const
{
  return this->m_plist->relatedPropertyType();
}

DescriptivePhraseType PropertyValuePhrase::propertyToPhraseType(PropertyType p)
{
  switch (p)
    {
  case FLOAT_PROPERTY:
    return FLOAT_PROPERTY_VALUE;
  case STRING_PROPERTY:
    return STRING_PROPERTY_VALUE;
  case INTEGER_PROPERTY:
    return INTEGER_PROPERTY_VALUE;
  case INVALID_PROPERTY:
  default:
    break;
    }
  return INVALID_DESCRIPTION;
}

  } // model namespace
} // smtk namespace
