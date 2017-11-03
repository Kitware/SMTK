//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/DescriptivePhrase.h"
#include "smtk/mesh/core/Collection.h"
#include "smtk/model/SubphraseGenerator.h"

namespace smtk
{
namespace model
{

unsigned int DescriptivePhrase::s_nextPhraseId = 0;

DescriptivePhrase::DescriptivePhrase()
  : m_type(INVALID_DESCRIPTION)
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

/// An efficient, but unwrappable, method that returns further phrases describing this one.
DescriptivePhrases& DescriptivePhrase::subphrases()
{
  this->buildSubphrases();
  return this->m_subphrases;
}

/// A wrappable version of subphrases().
DescriptivePhrases DescriptivePhrase::subphrases() const
{
  const_cast<DescriptivePhrase*>(this)->buildSubphrases();
  return this->m_subphrases;
}

/// Return the index of the given phrase in this instance's subphrases (or -1).
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

/// Return the index of the given EntityRef in this instance's subphrases (or -1).
int DescriptivePhrase::argFindChild(const EntityRef& child) const
{
  int i = 0;
  DescriptivePhrases::const_iterator it;
  for (it = this->m_subphrases.begin(); it != this->m_subphrases.end(); ++it, ++i)
  {
    if (it->get()->relatedEntity() == child)
      return i;
  }
  return -1;
}

/// Return the index of the given MeshSet in this instance's subphrases (or -1).
int DescriptivePhrase::argFindChild(const smtk::mesh::MeshSet& child) const
{
  if (child.is_empty())
  {
    return -1;
  }
  int i = 0;
  DescriptivePhrases::const_iterator it;
  for (it = this->m_subphrases.begin(); it != this->m_subphrases.end(); ++it, ++i)
  {
    if (it->get()->phraseType() == MESH_SUMMARY && !it->get()->relatedMesh().is_empty() &&
      it->get()->relatedMesh() == child)
      return i;
  }
  return -1;
}

/// Return the index of the given CollectionPtr in this instance's subphrases (or -1).
int DescriptivePhrase::argFindChild(const smtk::mesh::CollectionPtr& child) const
{
  if (!child)
  {
    return -1;
  }
  int i = 0;
  DescriptivePhrases::const_iterator it;
  for (it = this->m_subphrases.begin(); it != this->m_subphrases.end(); ++it, ++i)
  {
    if (it->get()->phraseType() == MESH_SUMMARY && it->get()->relatedMeshCollection() &&
      it->get()->relatedMeshCollection()->entity() == child->entity())
      return i;
  }
  return -1;
}

/// Return the index of the given property (name, type) in this instance's subphrases (or -1).
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

/// Find the subphrase generator to use. It may be held by a parent phrase.
SubphraseGeneratorPtr DescriptivePhrase::findDelegate()
{
  DescriptivePhrasePtr phr;
  for (phr = shared_from_this(); phr && !phr->m_delegate; phr = phr->parent())
    /* do nothing */;
  return phr ? phr->m_delegate : SubphraseGeneratorPtr();
}

/// Build (if required) the cached subphrases of this phrase.
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

/// Return whether this is a property value phrase.
bool DescriptivePhrase::isPropertyValueType() const
{
  DescriptivePhraseType phType = this->phraseType();
  return phType == FLOAT_PROPERTY_VALUE || phType == INTEGER_PROPERTY_VALUE ||
    phType == STRING_PROPERTY_VALUE;
}

bool DescriptivePhrase::compareByModelInfo(
  const DescriptivePhrasePtr& a, const DescriptivePhrasePtr& b)
{
  static const int sortOrder[] = {
    1,  // ENTITY_LIST
    0,  // ENTITY_SUMMARY
    2,  // ARRANGEMENT_LIST
    3,  // ATTRIBUTE_LIST
    4,  // MODEL_INCLUDES_ENTITY
    5,  // MODEL_EMBEDDED_IN_MODEL
    6,  // CELL_INCLUDES_CELL
    7,  // CELL_EMBEDDED_IN_CELL
    8,  // CELL_HAS_SHELL
    9,  // CELL_HAS_USE
    10, // SHELL_HAS_CELL
    11, // SHELL_HAS_USE
    12, // USE_HAS_CELL
    13, // USE_HAS_SHELL
    15, // FLOAT_PROPERTY_LIST
    16, // STRING_PROPERTY_LIST
    14, // INTEGER_PROPERTY_LIST
    18, // ENTITY_HAS_FLOAT_PROPERTY
    19, // ENTITY_HAS_STRING_PROPERTY
    17, // ENTITY_HAS_INTEGER_PROPERTY
    20, // ENTITY_HAS_ATTRIBUTE
    22, // FLOAT_PROPERTY_VALUE
    23, // STRING_PROPERTY_VALUE
    21, // INTEGER_PROPERTY_VALUE
    24, // ENTITY_HAS_SUBPHRASES
    25  // INVALID_DESCRIPTION
  };

  // I. Sort by phrase type.
  DescriptivePhraseType pta = a->phraseType();
  DescriptivePhraseType ptb = b->phraseType();
  if (pta != ptb)
  {
    return sortOrder[pta] < sortOrder[ptb];
  }

  // II. Sort by entity type/dimension
  // II.a. Entity type
  BitFlags eta = a->relatedEntity().entityFlags() & ENTITY_MASK;
  BitFlags etb = b->relatedEntity().entityFlags() & ENTITY_MASK;
  if (eta != etb)
  {
    switch (eta)
    {
      case CELL_ENTITY: // 0x0100
        return etb == MODEL_ENTITY ? false : true;
      case USE_ENTITY: // 0x0200
        return etb == MODEL_ENTITY || etb < USE_ENTITY ? false : true;
      case SHELL_ENTITY: // 0x0400
        return etb == MODEL_ENTITY || etb < SHELL_ENTITY ? false : true;
      case GROUP_ENTITY: // 0x0800
        return etb == MODEL_ENTITY || etb < SHELL_ENTITY ? false : true;
      case MODEL_ENTITY: // 0x1000
        return true;
      case INSTANCE_ENTITY: // 0x2000
        return false;
      default:
        return eta < etb ? true : false;
    }
  }

  // II.b. Dimension
  eta = a->relatedEntity().entityFlags() & ANY_DIMENSION;
  etb = b->relatedEntity().entityFlags() & ANY_DIMENSION;
  if (eta != etb)
    return eta < etb;

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

} // model namespace
} // smtk namespace
