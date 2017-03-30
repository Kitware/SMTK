//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/PropertyListPhrase.h"
#include "smtk/model/PropertyValuePhrase.h"

#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/StringData.h"

namespace smtk
{
namespace model
{

PropertyListPhrase::PropertyListPhrase()
  : m_propertyType(INVALID_PROPERTY)
{
}

/// Initialize the list with information required to generate subphrases.
PropertyListPhrase::Ptr PropertyListPhrase::setup(
  const EntityRef& entity, PropertyType ptype, DescriptivePhrasePtr parnt)
{
  this->m_entity = entity;
  this->m_propertyType = ptype;
  this->DescriptivePhrase::setup(PropertyListPhrase::propertyToPhraseType(ptype), parnt);
  return shared_from_this();
}

/// Initialize the list with a subset \a pnames of the \a entity's properties.
PropertyListPhrase::Ptr PropertyListPhrase::setup(const EntityRef& entity, PropertyType ptype,
  const std::set<std::string>& pnames, DescriptivePhrasePtr parnt)
{
  this->m_entity = entity;
  this->m_propertyType = ptype;
  this->m_propertyNames = pnames;
  this->DescriptivePhrase::setup(PropertyListPhrase::propertyToPhraseType(ptype), parnt);
  return shared_from_this();
}
std::string PropertyListPhrase::title()
{
  std::ostringstream message;
  DescriptivePhrases::size_type sz = this->subphrases().size();
  message << sz << " "
          << (this->m_propertyType == FLOAT_PROPERTY
                 ? "floating-point"
                 : (this->m_propertyType == STRING_PROPERTY
                       ? "string"
                       : (this->m_propertyType == INTEGER_PROPERTY ? "integer" : "invalid")))
          << " " << (sz == 1 ? "property" : "properties");
  return message.str();
}

std::string PropertyListPhrase::subtitle()
{
  return std::string();
}

smtk::common::UUID PropertyListPhrase::relatedEntityId() const
{
  return this->m_entity.entity();
}

EntityRef PropertyListPhrase::relatedEntity() const
{
  return this->m_entity;
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

} // model namespace
} // smtk namespace
