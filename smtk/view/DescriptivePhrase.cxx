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

#include "smtk/view/Badge.h"
#include "smtk/view/BadgeSet.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"

#include "smtk/model/Entity.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

#include <algorithm>

using smtk::resource::ComponentPtr;

namespace smtk
{
namespace view
{

unsigned int DescriptivePhrase::s_nextPhraseId = 0;

DescriptivePhrase::DescriptivePhrase()
{
  m_phraseId = DescriptivePhrase::s_nextPhraseId++;
}

DescriptivePhrasePtr DescriptivePhrase::setup(DescriptivePhraseType ptype, Ptr parnt)
{
  m_parent = parnt;
  m_type = ptype;
  m_subphrasesBuilt = false;
  return shared_from_this();
}

DescriptivePhrasePtr DescriptivePhrase::setDelegate(SubphraseGeneratorPtr delegate)
{
  if (m_delegate != delegate)
  {
    m_delegate = delegate;
    // If we already have subphrases, we need to build a new set using
    // the new subphrase generator and replace as needed:
    if (this->areSubphrasesBuilt())
    {
      // If this phrase has no parent, assume it is the root
      // and do not rebuild its children. Instead, rebuild its
      // grandchildren -- the PhraseModel is responsible for
      // maintaining the children of the root phrase.
      if (!this->parent())
      {
        auto& children = this->subphrases();
        for (const auto& child : children)
        {
          if (child->areSubphrasesBuilt())
          {
            child->markDirty(true);
            child->buildSubphrases();
          }
        }
      }
      else
      {
        this->markDirty(true);
        this->buildSubphrases();
      }
    }
  }
  return shared_from_this();
}

bool DescriptivePhrase::setContent(PhraseContentPtr content)
{
  if (!content ^ !m_content)
  { // Only one is non-null
    m_content = content;
    return true;
  }
  // Now either both are null or both are non-null. If non-null, compare them by value:
  if (content)
  {
    if (*content == *m_content)
    {
      return false;
    }
  }
  m_content = content;
  // TODO: should we fetch the PhraseModel and call "modified"?
  return true;
}

PhraseContentPtr DescriptivePhrase::content() const
{
  return m_content;
}

DescriptivePhrase::Badges DescriptivePhrase::badges() const
{
  DescriptivePhrase::Badges result;
  auto model = this->phraseModel();
  if (model)
  {
    result = model->badges().badgesFor(this);
  }
  return result;
}

DescriptivePhrases& DescriptivePhrase::subphrases()
{
  this->buildSubphrases();
  return m_subphrases;
}

DescriptivePhrases DescriptivePhrase::subphrases() const
{
  const_cast<DescriptivePhrase*>(this)->buildSubphrases();
  return m_subphrases;
}

int DescriptivePhrase::argFindChild(const DescriptivePhrase* child) const
{
  int i = 0;
  DescriptivePhrases::const_iterator it;
  for (it = m_subphrases.begin(); it != m_subphrases.end(); ++it, ++i)
  {
    if (it->get() == child)
      return i;
  }
  return -1;
}

int DescriptivePhrase::argFindChild(const smtk::resource::ResourcePtr& child, bool onlyResource)
  const
{
  int i = 0;
  DescriptivePhrases::const_iterator it;
  for (it = m_subphrases.begin(); it != m_subphrases.end(); ++it, ++i)
  {
    auto* sp = it->get();
    if (sp->relatedResource() == child && (!onlyResource || !sp->relatedComponent()))
    {
      return i;
    }
  }
  return -1;
}

/// Return the index of the given Component pointer in this instance's subphrases (or -1).
int DescriptivePhrase::argFindChild(const smtk::resource::ComponentPtr& child) const
{
  int i = 0;
  DescriptivePhrases::const_iterator it;
  for (it = m_subphrases.begin(); it != m_subphrases.end(); ++it, ++i)
  {
    if (it->get()->relatedComponent() == child)
    {
      return i;
    }
  }
  return -1;
}

int DescriptivePhrase::argFindChild(
  const std::string& propName,
  smtk::resource::PropertyType propType) const
{
  (void)propType;
  int i = 0;
  DescriptivePhrases::const_iterator it;
  for (it = m_subphrases.begin(); it != m_subphrases.end(); ++it, ++i)
  {
    if (propName == (*it)->title() && (*it)->isPropertyValueType())
      return i;
  }
  return -1;
}

int DescriptivePhrase::indexInParent() const
{
  const DescriptivePhrasePtr prnt = this->parent();
  if (prnt)
  {
    return prnt->argFindChild(this);
  }
  return 0;
}

void DescriptivePhrase::index(std::vector<int>& idx) const
{
  idx.clear();
  const auto* self = this;
  while (self)
  {
    DescriptivePhrasePtr prnt = self->parent();
    if (prnt)
    {
      idx.push_back(self->indexInParent());
    }
    self = prnt.get();
  }
  // Now flip the path so that the root is at the beginning and
  // indexInParent() is a the end.
  std::reverse(idx.begin(), idx.end());
}

DescriptivePhrasePtr DescriptivePhrase::root() const
{
  Ptr self = const_cast<DescriptivePhrase*>(this)->shared_from_this();
  Ptr prnt;
  while ((prnt = self->parent()))
  {
    self = prnt;
  }
  return self;
}

DescriptivePhrasePtr DescriptivePhrase::relative(const std::vector<int>& relativePath) const
{
  Ptr result = const_cast<DescriptivePhrase*>(this)->shared_from_this();
  for (auto entry : relativePath)
  {
    if (entry < 0)
    {
      for (; entry < 0 && result; ++entry)
      {
        result = result->parent();
      }
      if (!result)
      {
        return result;
      }
    }
    else
    {
      DescriptivePhrases children = result->subphrases();
      if (entry >= static_cast<int>(children.size()))
      {
        result = Ptr();
        return result;
      }
      result = children[entry];
    }
  }
  return result;
}

DescriptivePhrasePtr DescriptivePhrase::at(const std::vector<int>& absolutePath) const
{
  auto rr = this->root();
  if (rr)
  {
    return rr->relative(absolutePath);
  }
  return rr;
}

smtk::common::UUID DescriptivePhrase::relatedComponentId() const
{
  return this->relatedComponent() ? this->relatedComponent()->id() : smtk::common::UUID::null();
}

SubphraseGeneratorPtr DescriptivePhrase::findDelegate() const
{
  ConstPtr phr = shared_from_this();
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
  this->index(indices); // Initialize the starting index for traversal.
  this->visitChildrenInternal(fn, indices);
}

PhraseModelPtr DescriptivePhrase::phraseModel() const
{
  auto delegate = this->findDelegate();
  if (delegate)
  {
    return delegate->model();
  }
  return PhraseModelPtr();
}

bool DescriptivePhrase::hasChildren() const
{
  // Do we have sub phrases already
  if (m_subphrasesBuilt)
  {
    return !m_subphrases.empty();
  }
  // Ok we need to ask the subphrase generator
  auto delegate = this->findDelegate();
  if (!delegate)
  {
    return false; // No generator so no children
  }
  return delegate->hasChildren(*this);
}

static int modelEntitySortOrder(smtk::model::EntityPtr ent)
{
  if (!ent)
  {
    // Put non-model entities at the end of the list.
    return 0x8000;
  }

  // If model entity type is same, we want the highest
  // dimension entities listed first (i.e., volumes, faces,
  // edges, verts):
  int db = 0x1f - ent->dimensionBits(); // at most 0x1f

  if (ent->isCellEntity())
  {
    return 0x80 + db;
  }
  else if (ent->isGroup())
  {
    return 0x40 + db;
  }
  else if (ent->isModel())
  {
    return 0x20 + db;
  }
  else if (ent->isAuxiliaryGeometry())
  {
    return 0x100 + db;
  }
  else if (ent->isInstance())
  {
    return 0x120 + db;
  }
  else if (ent->isShellEntity())
  {
    return 0x140 + db;
  }
  else if (ent->isUseEntity())
  {
    return 0x160 + db;
  }
  else if (ent->isConcept())
  {
    return 0x60 + db;
  }
  // Put unknown model stuff at the end of the list:
  return 0x4000 + db;
}

bool DescriptivePhrase::compareByTypeThenTitle(
  const DescriptivePhrasePtr& a,
  const DescriptivePhrasePtr& b)
{
  static constexpr int sortOrder[] = {
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

  if (!a && b)
  {
    return false;
  }
  if (!b && a)
  {
    return true;
  }
  if (!a && !b)
  {
    return false;
  }

  // I. Sort by phrase type.
  DescriptivePhraseType pta = a->phraseType();
  DescriptivePhraseType ptb = b->phraseType();
  if (pta != ptb)
  {
    return sortOrder[static_cast<int>(pta)] < sortOrder[static_cast<int>(ptb)];
  }

  // II. Sort by related resource, so that things in the same file appear together
  smtk::resource::Resource* rpa = a->relatedRawResource();
  smtk::resource::Resource* rpb = b->relatedRawResource();
  if (rpa != rpb)
  {
    if (rpa && rpb)
    {
      // Sort by resource _type_ first, so files of the same type appear together
      if (rpa->typeName() < rpb->typeName())
      {
        return true;
      }
      if (rpa->typeName() > rpb->typeName())
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
      return false;
    }
  }

  // III. Sort by component type (if component is present)
  smtk::resource::Component* cpa = a->relatedRawComponent();
  smtk::resource::Component* cpb = b->relatedRawComponent();
  if (cpa != cpb)
  {
    if (cpa && cpb)
    {
      // Note that since we compared resource types above,
      // both components must be of the same resource-type.
      // So, if mea != nullptr, then meb != nullptr, too.
      // Currently, mesh and attribute resources have only 1 component type,
      // so we don't need to discriminate here.
      int mea = modelEntitySortOrder(cpa->as<smtk::model::Entity>());
      int meb = modelEntitySortOrder(cpb->as<smtk::model::Entity>());
      if (mea != meb)
      {
        return mea < meb;
      }
    }
    else if (cpa)
    {
      return false; // a is component, b is resource
    }
    else // cpb, but !cpa
    {
      return true; // a is resource, b is component
    }
  }

  // IV. Sort by title, with care taken when differences are numeric values.
  return compareByTitle(a, b);
}

bool DescriptivePhrase::compareByTitle(const DescriptivePhrasePtr& a, const DescriptivePhrasePtr& b)
{
  std::string ta(a->title());
  std::string tb(b->title());

  if (ta.empty() && tb.empty())
  {
    return false;
  }

  if (ta.empty())
  {
    return true;
  }

  if (tb.empty())
  {
    return false;
  }

  std::string::size_type minlen = ta.size() < tb.size() ? ta.size() : tb.size();
  std::string::size_type i;
  for (i = 0; i < minlen; ++i)
    if (ta[i] != tb[i])
      break; // Stop at the first difference between ta and tb.

  // Shorter strings are less than longer versions with the same start:
  if (i == minlen)
    return ta.size() < tb.size();

  // Both ta & tb have some character present and different.
  bool da = isdigit(ta[i]) != 0;
  bool db = isdigit(tb[i]) != 0;
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
  smtk::view::PhraseContent* c0 = this->content().get();
  smtk::view::PhraseContent* c1 = other.content().get();
  bool same = (*c0 == *c1);
  return same &&
    // You wouldn't think typeid(baseClassReference) would work, but it does virtual lookup:
    typeid(*this) == typeid(other) && m_parent.lock() == other.m_parent.lock() &&
    m_type == other.m_type && m_delegate == other.m_delegate;
}

bool DescriptivePhrase::operator!=(const DescriptivePhrase& other) const
{
  return !(*this == other);
}

void DescriptivePhrase::reparent(const DescriptivePhrasePtr& nextParent)
{
  m_parent = nextParent;
}

void DescriptivePhrase::buildSubphrases()
{
  if (!m_subphrasesBuilt)
  {
    m_subphrasesBuilt = true;
    SubphraseGeneratorPtr delegate = this->findDelegate();
    if (delegate)
    {
      DescriptivePhrases next = delegate->subphrases(shared_from_this());
      PhraseModelPtr phraseModel = delegate->model();
      if (phraseModel)
      {
        phraseModel->updateChildren(shared_from_this(), next, this->index());
      }
      else
      {
        m_subphrases = next;
      }
    }
  }
}

void DescriptivePhrase::manuallySetSubphrases(const DescriptivePhrases& children, bool notify)
{
  bool didNotify = false;
  if (notify)
  {
    auto delegate = this->findDelegate();
    if (delegate)
    {
      auto model = delegate->model();
      if (model)
      {
        m_subphrasesBuilt = true; // Prevent the generator from running inside updateChildren().
        auto mutableChildren = children;
        model->updateChildren(shared_from_this(), mutableChildren, this->index());
        didNotify = true;
      }
    }
  }
  if (!didNotify)
  {
    m_subphrases = children;
    m_subphrasesBuilt = true;
  }
}

int DescriptivePhrase::visitChildrenInternal(Visitor fn, std::vector<int>& indices)
{
  int traverse = 0;
  if (this->areSubphrasesBuilt())
  {
    DescriptivePhrases list = this->subphrases();
    indices.insert(indices.end(), 0);
    for (const auto& entry : list)
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

} // namespace view
} // namespace smtk
