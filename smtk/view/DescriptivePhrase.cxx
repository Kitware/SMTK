//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/DescriptivePhrase.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

#include "smtk/view/SubphraseGenerator.h"

using smtk::resource::Component;
using smtk::resource::ComponentPtr;

namespace smtk
{
namespace view
{

unsigned int DescriptivePhrase::s_nextPhraseId = 0;

DescriptivePhrase::DescriptivePhrase()
  : m_type(DescriptivePhraseType::INVALID_DESCRIPTION)
  , m_subphrasesBuilt(false)
{
  this->m_phraseId = DescriptivePhrase::s_nextPhraseId++;
}

DescriptivePhrasePtr DescriptivePhrase::setup(DescriptivePhraseType ptype, Ptr parnt)
{
  this->m_parent = parnt;
  this->m_type = ptype;
  this->m_subphrasesBuilt = false;
  return shared_from_this();
}

DescriptivePhrasePtr DescriptivePhrase::setDelegate(SubphraseGeneratorPtr delegate)
{
  this->m_delegate = delegate;
  return shared_from_this();
}

DescriptivePhrases& DescriptivePhrase::subphrases()
{
  this->buildSubphrases();
  return this->m_subphrases;
}

DescriptivePhrases DescriptivePhrase::subphrases() const
{
  const_cast<DescriptivePhrase*>(this)->buildSubphrases();
  return this->m_subphrases;
}

int DescriptivePhrase::argFindChild(const DescriptivePhrase* child) const
{
  int i = 0;
  DescriptivePhrases::const_iterator it;
  for (it = this->m_subphrases.begin(); it != this->m_subphrases.end(); ++it, ++i)
  {
    if (it->get() == child)
      return i;
  }
  return -1;
}

int DescriptivePhrase::argFindChild(smtk::resource::ResourcePtr child, bool onlyResource) const
{
  int i = 0;
  DescriptivePhrases::const_iterator it;
  for (it = this->m_subphrases.begin(); it != this->m_subphrases.end(); ++it, ++i)
  {
    auto sp = it->get();
    if (sp->relatedResource() == child && (!onlyResource || !sp->relatedComponent()))
    {
      return i;
    }
  }
  return -1;
}

/// Return the index of the given Component pointer in this instance's subphrases (or -1).
int DescriptivePhrase::argFindChild(smtk::resource::ComponentPtr child) const
{
  int i = 0;
  DescriptivePhrases::const_iterator it;
  for (it = this->m_subphrases.begin(); it != this->m_subphrases.end(); ++it, ++i)
  {
    if (it->get()->relatedComponent() == child)
    {
      return i;
    }
  }
  return -1;
}

int DescriptivePhrase::argFindChild(
  const std::string& propName, smtk::resource::PropertyType propType) const
{
  (void)propType;
  int i = 0;
  DescriptivePhrases::const_iterator it;
  for (it = this->m_subphrases.begin(); it != this->m_subphrases.end(); ++it, ++i)
  {
    if (propName == (*it)->title() && (*it)->isPropertyValueType())
      return i;
  }
  return -1;
}

/// Return the index of this phrase in its parent instance's subphrases (or -1).
int DescriptivePhrase::indexInParent() const
{
  const DescriptivePhrasePtr prnt = this->parent();
  if (prnt)
  {
    return prnt->argFindChild(this);
  }
  return 0;
}

smtk::common::UUID DescriptivePhrase::relatedComponentId() const
{
  return this->relatedComponent() ? this->relatedComponent()->id() : smtk::common::UUID::null();
}

SubphraseGeneratorPtr DescriptivePhrase::findDelegate()
{
  DescriptivePhrasePtr phr;
  for (phr = shared_from_this(); phr && !phr->m_delegate; phr = phr->parent())
    /* do nothing */;
  return phr ? phr->m_delegate : SubphraseGeneratorPtr();
}

bool DescriptivePhrase::isPropertyValueType() const
{
  DescriptivePhraseType phType = this->phraseType();
  return phType == DescriptivePhraseType::FLOAT_PROPERTY_VALUE ||
    phType == DescriptivePhraseType::INTEGER_PROPERTY_VALUE ||
    phType == DescriptivePhraseType::STRING_PROPERTY_VALUE;
}

void DescriptivePhrase::visitChildren(Visitor fn)
{
  std::vector<int> indices;
  this->visitChildrenInternal(fn, indices);
}

bool DescriptivePhrase::compareByTypeThenTitle(
  const DescriptivePhrasePtr& a, const DescriptivePhrasePtr& b)
{
  static const int sortOrder[] = {
    0, // RESOURCE_SUMMARY
    2, // RESOURCE_LIST
    1, // COMPONENT_SUMMARY
    3, // COMPONENT_LIST
    7, // PROPERTY_LIST
    5, // FLOAT_PROPERTY_VALUE
    6, // STRING_PROPERTY_VALUE
    4, // INTEGER_PROPERTY_VALUE
    8, // LIST (free form)
    9  // INVALID_DESCRIPTION
  };

  // I. Sort by phrase type.
  DescriptivePhraseType pta = a->phraseType();
  DescriptivePhraseType ptb = b->phraseType();
  if (pta != ptb)
  {
    return sortOrder[static_cast<int>(pta)] < sortOrder[static_cast<int>(ptb)];
  }

  // II. Sort by related resource, so that things in the same file appear together
  smtk::resource::ResourcePtr rpa = a->relatedResource();
  smtk::resource::ResourcePtr rpb = b->relatedResource();
  if (rpa != rpb)
  {
    if (rpa && rpb)
    {
      // Sort by resource _type_ first, so files of the same type appear together
      if (rpa->uniqueName() < rpb->uniqueName())
      {
        return true;
      }
      if (rpa->uniqueName() > rpb->uniqueName())
      {
        return false;
      }
      // Then sort by resource url because resources don't have a name.
      return rpa->location() < rpb->location();
    }
    else if (rpa)
    {
      return false;
    }
    else // (!rpb || (!rpa && !rpb))
    {
      return true;
    }
  }

  // III. Sort by title, with care taken when differences are numeric values.
  return compareByTitle(a, b);
}

bool DescriptivePhrase::compareByTitle(const DescriptivePhrasePtr& a, const DescriptivePhrasePtr& b)
{
  std::string ta(a->title());
  std::string tb(b->title());
  if (ta.empty())
    return true;
  if (tb.empty())
    return false;
  std::string::size_type minlen = ta.size() < tb.size() ? ta.size() : tb.size();
  std::string::size_type i;
  for (i = 0; i < minlen; ++i)
    if (ta[i] != tb[i])
      break; // Stop at the first difference between ta and tb.

  // Shorter strings are less than longer versions with the same start:
  if (i == minlen)
    return ta.size() < tb.size() ? true : false;

  // Both ta & tb have some character present and different.
  bool da = isdigit(ta[i]) ? true : false;
  bool db = isdigit(tb[i]) ? true : false;
  if (da && !db)
    return true; // digits come before other things
  if (!da && db)
    return false; // non-digits come after digits
  if (!da && !db)
    return ta[i] < tb[i];
  // Now, both ta and tb differ with some numeric value.
  // Convert to a number and compare the numbers.
  double na = atof(ta.substr(i).c_str());
  double nb = atof(tb.substr(i).c_str());
  return na < nb;
}

bool DescriptivePhrase::operator==(const DescriptivePhrase& other) const
{
  return
    // You wouldn't think typeid(baseClassReference) would work, but it does virtual lookup:
    typeid(*this) == typeid(other) && this->m_parent.lock() == other.m_parent.lock() &&
    this->m_type == other.m_type && this->m_delegate == other.m_delegate;
}

bool DescriptivePhrase::operator!=(const DescriptivePhrase& other) const
{
  return !(*this == other);
}

void DescriptivePhrase::buildSubphrases()
{
  if (!this->m_subphrasesBuilt)
  {
    this->m_subphrasesBuilt = true;
    SubphraseGeneratorPtr delegate = this->findDelegate();
    if (delegate)
      this->m_subphrases = delegate->subphrases(shared_from_this());
  }
}

int DescriptivePhrase::visitChildrenInternal(Visitor fn, std::vector<int>& indices)
{
  int traverse = 0;
  if (this->areSubphrasesBuilt())
  {
    DescriptivePhrases list = this->subphrases();
    indices.insert(indices.end(), 0);
    for (auto entry : list)
    {
      if (entry)
      {
        traverse = fn(entry, indices); // Only call fn when entry is non-null?
        if (traverse == 0)
        {
          traverse = entry->visitChildrenInternal(fn, indices);
        }
        if (traverse > 1)
        {
          break;
        }
        // Increment counter
        ++indices.back();
      }
    }
    indices.pop_back();
  }
  return traverse;
}

} // view namespace
} // smtk namespace
