//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_ReferenceItemDefinition_txx
#define smtk_attribute_ReferenceItemDefinition_txx

#include "smtk/attribute/ReferenceItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"

#include "smtk/resource/Container.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include <cassert>

namespace smtk
{
namespace attribute
{

ReferenceItemDefinition::ReferenceItemDefinition(const std::string& sname)
  : Superclass(sname)
{
  m_numberOfRequiredValues = 1;
  m_useCommonLabel = false;
  m_isExtensible = false;
  m_maxNumberOfValues = 0;
  m_lockType = smtk::resource::LockType::Write;
  m_role = smtk::attribute::Resource::ReferenceRole;
  m_holdReference = false;
  m_onlyResources = false;
}

ReferenceItemDefinition::~ReferenceItemDefinition() = default;

bool ReferenceItemDefinition::setAcceptsEntries(
  const std::string& typeName, const std::string& filter, bool accept)
{
  if (accept)
  {
    m_acceptable.insert(std::make_pair(typeName, filter));
    return true;
  }
  else
  {
    auto range = m_acceptable.equal_range(typeName);
    auto found = std::find_if(
      range.first, range.second, [&](decltype(*range.first) it) { return it.second == filter; });

    if (found != m_acceptable.end())
    {
      m_acceptable.erase(found);
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool ReferenceItemDefinition::isValueValid(resource::ConstPersistentObjectPtr entity) const
{
  bool ok = false;
  if (!entity)
  {
    return ok;
  }

  const smtk::resource::Resource* rsrc;
  const smtk::resource::Component* comp;
  if ((rsrc = dynamic_cast<const smtk::resource::Resource*>(entity.get())))
  {
    ok = this->checkResource(*rsrc);
  }
  else if ((comp = dynamic_cast<const smtk::resource::Component*>(entity.get())))
  {
    ok = this->checkComponent(*comp);
  }
  return ok;
}

std::size_t ReferenceItemDefinition::numberOfRequiredValues() const
{
  return m_numberOfRequiredValues;
}

void ReferenceItemDefinition::setNumberOfRequiredValues(std::size_t esize)
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

void ReferenceItemDefinition::setMaxNumberOfValues(std::size_t maxNum)
{
  m_maxNumberOfValues = maxNum;
}

bool ReferenceItemDefinition::hasValueLabels() const
{
  return !m_valueLabels.empty();
}

std::string ReferenceItemDefinition::valueLabel(std::size_t i) const
{
  if (m_useCommonLabel)
  {
    assert(!m_valueLabels.empty());
    return m_valueLabels[0];
  }
  if (!m_valueLabels.empty())
  {
    assert(m_valueLabels.size() > i);
    return m_valueLabels[i];
  }
  return "";
}

void ReferenceItemDefinition::setValueLabel(std::size_t i, const std::string& elabel)
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
  m_valueLabels[i] = elabel;
}

void ReferenceItemDefinition::setCommonValueLabel(const std::string& elabel)
{
  m_useCommonLabel = true;
  m_valueLabels.resize(1);
  m_valueLabels[0] = elabel;
}

bool ReferenceItemDefinition::usingCommonLabel() const
{
  return m_useCommonLabel;
}

smtk::attribute::ItemPtr ReferenceItemDefinition::buildItem(
  Attribute* owningAttribute, int itemPosition) const
{
  return smtk::attribute::ItemPtr(new ReferenceItem(owningAttribute, itemPosition));
}

smtk::attribute::ItemPtr ReferenceItemDefinition::buildItem(
  Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new ReferenceItem(owningItem, itemPosition, subGroupPosition));
}

smtk::attribute::ItemDefinitionPtr ReferenceItemDefinition::createCopy(
  smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  (void)info;
  auto copy = ReferenceItemDefinition::New(this->name());
  this->copyTo(copy);
  return copy;
}

void ReferenceItemDefinition::copyTo(Ptr dest) const
{
  if (!dest)
  {
    return;
  }

  Superclass::copyTo(dest);

  dest->setNumberOfRequiredValues(m_numberOfRequiredValues);
  dest->setMaxNumberOfValues(m_maxNumberOfValues);
  dest->setIsExtensible(m_isExtensible);
  for (auto& acceptable : m_acceptable)
  {
    dest->setAcceptsEntries(acceptable.first, acceptable.second, true);
  }
  if (m_useCommonLabel)
  {
    dest->setCommonValueLabel(m_valueLabels[0]);
  }
  else if (this->hasValueLabels())
  {
    std::size_t ii;
    for (ii = 0; ii < m_valueLabels.size(); ++ii)
    {
      dest->setValueLabel(ii, m_valueLabels[ii]);
    }
  }
}

bool ReferenceItemDefinition::checkResource(const smtk::resource::Resource& rsrc) const
{
  // If there are no filter values, then we accept all resources.
  if (m_acceptable.empty())
  {
    return true;
  }

  // TODO:
  // Queries to filter resources as acceptable have not been implemented.
  // See smtk::attribute::ComponentItemDefinition::isValueValid() for
  // a pattern to follow when implementing this.
  //
  // For now all we do is test the resource names in m_acceptable
  // to see if any are exact matches for rsrc.

  // For every element in the filter map...
  for (auto& acceptable : m_acceptable)
  {
    // ...we check if the resource in question is of that type. Acceptable
    // entries for resources do not have a filter string, so we check that
    // the filter string is empty.
    if ((acceptable.second.empty() || m_onlyResources) && rsrc.isOfType(acceptable.first))
    {
      return true;
    }
  }

  return false;
}

bool ReferenceItemDefinition::checkComponent(const smtk::resource::Component& comp) const
{
  // All components are required to have resources in order to be valid.
  auto rsrc = comp.resource();
  if (!rsrc)
  {
    return false;
  }

  // If there are no filter values, then we accept all components.
  if (m_acceptable.empty())
  {
    return true;
  }

  // For every element in the filter map...
  for (auto& acceptable : m_acceptable)
  {
    // ...ask (a) if the filter explicitly rejects components, (b) if our
    // resource is of the right type, and (b) if its associated filter accepts
    // the component.
    if (!m_onlyResources && rsrc->isOfType(acceptable.first) &&
      rsrc->queryOperation(acceptable.second)(comp))
    {
      return true;
    }
  }

  return false;
}
}
}
#endif
