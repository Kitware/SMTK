//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/Resource.h"

#include <queue>

using namespace smtk::attribute;

RefItemDefinition::RefItemDefinition(const std::string& myName)
  : ItemDefinition(myName)
  , m_definition()
{
  m_useCommonLabel = false;
  m_numberOfRequiredValues = 1;
}

RefItemDefinition::~RefItemDefinition()
{
}

Item::Type RefItemDefinition::type() const
{
  return Item::AttributeRefType;
}

bool RefItemDefinition::isValueValid(smtk::attribute::AttributePtr att) const
{
  if (!att)
  {
    return true;
  }
  if (m_definition.lock())
  {
    return att->isA(m_definition.lock());
  }
  return true;
}

smtk::attribute::ItemPtr RefItemDefinition::buildItem(
  Attribute* owningAttribute, int itemPosition) const
{
  return smtk::attribute::ItemPtr(new RefItem(owningAttribute, itemPosition));
}

smtk::attribute::ItemPtr RefItemDefinition::buildItem(
  Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new RefItem(owningItem, itemPosition, subGroupPosition));
}

void RefItemDefinition::setNumberOfRequiredValues(std::size_t esize)
{
  if (esize == m_numberOfRequiredValues)
  {
    return;
  }
  m_numberOfRequiredValues = esize;
  if (!m_useCommonLabel)
  {
    m_valueLabels.resize(esize);
  }
}

void RefItemDefinition::setValueLabel(std::size_t element, const std::string& elabel)
{
  if (m_numberOfRequiredValues == 0)
  {
    return;
  }
  if (m_valueLabels.size() != m_numberOfRequiredValues)
  {
    m_valueLabels.resize(m_numberOfRequiredValues);
  }
  m_useCommonLabel = false;
  m_valueLabels[element] = elabel;
}

void RefItemDefinition::setCommonValueLabel(const std::string& elabel)
{
  if (m_valueLabels.size() != 1)
  {
    m_valueLabels.resize(1);
  }
  m_useCommonLabel = true;
  m_valueLabels[0] = elabel;
}

std::string RefItemDefinition::valueLabel(std::size_t element) const
{
  if (m_useCommonLabel)
  {
    return m_valueLabels[0];
  }
  if (m_valueLabels.size())
  {
    return m_valueLabels[element];
  }
  return ""; // If we threw execeptions this method could return const string &
}

smtk::attribute::ItemDefinitionPtr smtk::attribute::RefItemDefinition::createCopy(
  smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  std::size_t i;

  smtk::attribute::RefItemDefinitionPtr newRef =
    smtk::attribute::RefItemDefinition::New(this->name());
  ItemDefinition::copyTo(newRef);

  // Set attributeDefinition (if possible)
  if (this->attributeDefinition())
  {
    std::string typeStr = this->attributeDefinition()->type();
    smtk::attribute::DefinitionPtr def = info.ToResource.findDefinition(typeStr);
    if (def)
    {
      newRef->setAttributeDefinition(def);
    }
    else
    {
      std::cout << "Adding definition \"" << typeStr << "\" to copy-definition queue" << std::endl;
      info.UnresolvedRefItems.push(std::make_pair(typeStr, newRef));
    }
  }

  newRef->setNumberOfRequiredValues(m_numberOfRequiredValues);

  // Labels
  if (m_useCommonLabel)
  {
    newRef->setCommonValueLabel(m_valueLabels[0]);
  }
  else if (this->hasValueLabels())
  {
    for (i = 0; i < m_valueLabels.size(); ++i)
    {
      newRef->setValueLabel(i, m_valueLabels[i]);
    }
  }

  return newRef;
}
