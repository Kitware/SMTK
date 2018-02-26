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
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/ReferenceItem.h"

#include "smtk/resource/Container.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include <cassert>

namespace smtk
{
namespace attribute
{

template <typename T>
ReferenceItemDefinition<T>::ReferenceItemDefinition(const std::string& sname)
  : Superclass(sname)
{
  m_numberOfRequiredValues = 1;
  m_useCommonLabel = false;
  m_isExtensible = false;
  m_maxNumberOfValues = 0;
  m_isWritable = true;
}

template <typename T>
ReferenceItemDefinition<T>::~ReferenceItemDefinition()
{
}

template <typename T>
bool ReferenceItemDefinition<T>::setAcceptsEntries(
  const std::string& uniqueName, const std::string& filter, bool accept)
{
  if (accept)
  {
    m_acceptable.insert(std::make_pair(uniqueName, filter));
    return true;
  }
  else
  {
    auto range = m_acceptable.equal_range(uniqueName);
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

template <typename T>
std::size_t ReferenceItemDefinition<T>::numberOfRequiredValues() const
{
  return m_numberOfRequiredValues;
}

template <typename T>
void ReferenceItemDefinition<T>::setNumberOfRequiredValues(std::size_t esize)
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

template <typename T>
void ReferenceItemDefinition<T>::setMaxNumberOfValues(std::size_t maxNum)
{
  m_maxNumberOfValues = maxNum;
}

template <typename T>
bool ReferenceItemDefinition<T>::hasValueLabels() const
{
  return !m_valueLabels.empty();
}

template <typename T>
std::string ReferenceItemDefinition<T>::valueLabel(std::size_t i) const
{
  if (m_useCommonLabel)
  {
    assert(!m_valueLabels.empty());
    return m_valueLabels[0];
  }
  if (m_valueLabels.size())
  {
    assert(m_valueLabels.size() > i);
    return m_valueLabels[i];
  }
  return "";
}

template <typename T>
void ReferenceItemDefinition<T>::setValueLabel(std::size_t i, const std::string& elabel)
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

template <typename T>
void ReferenceItemDefinition<T>::setCommonValueLabel(const std::string& elabel)
{
  m_useCommonLabel = true;
  m_valueLabels.resize(1);
  m_valueLabels[0] = elabel;
}

template <typename T>
bool ReferenceItemDefinition<T>::usingCommonLabel() const
{
  return m_useCommonLabel;
}

template <typename T>
void ReferenceItemDefinition<T>::copyTo(Ptr dest) const
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
}
}
#endif
