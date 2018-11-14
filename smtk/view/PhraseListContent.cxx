//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/PhraseListContent.h"

#include "smtk/view/DescriptivePhrase.h"

#include "smtk/model/EntityRef.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace view
{

PhraseListContent::PhraseListContent()
  : m_mutability(0)
  , m_commonFlags(smtk::model::INVALID)
  , m_unionFlags(0)
{
  // only color is mutable
}

PhraseListContent::Ptr PhraseListContent::setup(DescriptivePhrase::Ptr parent,
  smtk::model::BitFlags commonFlags, smtk::model::BitFlags unionFlags, int mutability)
{
  this->setLocation(parent);
  this->setModelFlags(commonFlags, unionFlags);
  m_mutability = mutability;
  return shared_from_this();
}

DescriptivePhrasePtr PhraseListContent::createPhrase(DescriptivePhrasePtr parent,
  smtk::model::BitFlags commonFlags, smtk::model::BitFlags unionFlags, int mutability,
  const DescriptivePhrases& children)
{
  (void)children;

  auto result = DescriptivePhrase::create()->setup(DescriptivePhraseType::COMPONENT_LIST, parent);
  auto content = PhraseListContent::create()->setup(result, commonFlags, unionFlags, mutability);
  content->setLocation(result);
  result->setContent(content);
  //result->manuallySetSubphrases(children, /*notify*/ false);
  return result;
}

bool PhraseListContent::displayable(ContentType attr) const
{
  switch (attr)
  {
    case PhraseContent::TITLE:
    case PhraseContent::SUBTITLE:
      return true;
    case PhraseContent::VISIBILITY:
    case PhraseContent::COLOR:
    case PhraseContent::ICON:
      break;
  }
  return false;
}

std::string PhraseListContent::stringValue(ContentType attr) const
{
  switch (attr)
  {
    case PhraseContent::TITLE:
      if (m_title.empty())
      {
        m_title = this->generateTitle(m_commonFlags, m_unionFlags);
      }
      return m_title;
      break;
    case PhraseContent::SUBTITLE:
      return std::string();
      break;

    // We will not provide strings for these:
    case PhraseContent::COLOR:
    case PhraseContent::VISIBILITY:
    case PhraseContent::ICON:
    default:
      break;
  }
  return std::string();
}

int PhraseListContent::flagValue(ContentType attr) const
{
  switch (attr)
  {
    case PhraseContent::COLOR:
    case PhraseContent::TITLE:
    case PhraseContent::SUBTITLE:
    case PhraseContent::VISIBILITY:
    case PhraseContent::ICON:
    // This should return non-default values once we allow icons to be registered
    // for components by their metadata.
    default:
      break;
  }
  return -1;
}

resource::FloatList PhraseListContent::colorValue(ContentType attr) const
{
  switch (attr)
  {
    case PhraseContent::COLOR:
      // Here is where we could look up a default color based on m_unionFlags/m_commonFlags.
      // This seems application- or context-specific, so perhaps it is best done in a decorator?
      break;
    case PhraseContent::TITLE:
    case PhraseContent::SUBTITLE:
    case PhraseContent::VISIBILITY:
    case PhraseContent::ICON:
    default:
      break;
  }
  smtk::resource::FloatList rgba(4, -1);
  return rgba;
}

bool PhraseListContent::editStringValue(ContentType attr, const std::string& val)
{
  // This should create and call a "set entity property" operator on the
  // related component's name for attr == TITLE.
  (void)attr;
  (void)val;
  return false;
}

bool PhraseListContent::editFlagValue(ContentType attr, int val)
{
  (void)attr;
  (void)val;
  return false;
}

bool PhraseListContent::editColorValue(ContentType attr, const resource::FloatList& val)
{
  // This should create and call a "set entity property" operator on the
  // related component's color for attr == COLOR.
  (void)attr;
  (void)val;
  return false;
}

/// The list of entities to be presented.
smtk::resource::ComponentArray PhraseListContent::relatedComponents() const
{
  smtk::resource::ComponentArray result;

  auto parent = this->location();
  if (!parent)
  {
    return result;
  }

  auto phrases = parent->subphrases();
  for (auto phrase : phrases)
  {
    result.push_back(phrase->relatedComponent());
  }
  return result;
}

/**\brief Inform the descriptor what bits in entityFlags() are set across the list.
  *
  * The \a commonFlags argument is the logical-AND of all entityFlags()
  * of all entities in the list. The \a unionFlags argument is the
  * logical-OR of all entityFlags().
  *
  * These are used by PhraseListContent to choose an appropriate summary
  * description of the entities.
  */
void PhraseListContent::setModelFlags(
  smtk::model::BitFlags commonFlags, smtk::model::BitFlags unionFlags)
{
  m_commonFlags = commonFlags;
  m_unionFlags = unionFlags;
}

void PhraseListContent::setMutability(int whatsMutable)
{
  m_mutability = whatsMutable;
}

bool PhraseListContent::operator==(const PhraseContent& other) const
{
  auto otherList(static_cast<const PhraseListContent&>(other));
  return this->equalTo(other) && (this->location() == otherList.location()) &&
    m_commonFlags == otherList.m_commonFlags && m_unionFlags == otherList.m_unionFlags;
}

std::string PhraseListContent::generateTitle(
  smtk::model::BitFlags& fCommon, smtk::model::BitFlags& fUnion) const
{
  auto phrase = this->location();
  if (!phrase)
  {
    return std::string();
  }

  // TODO: Handle homogenous and heterogenous subphrase lists
  //       the way its counterpart in smtk::model does.
  std::ostringstream message;
  smtk::resource::ComponentArray::size_type sz = phrase->subphrases().size();
  std::map<std::string, smtk::model::BitFlags> flagUnion;
  std::map<std::string, smtk::model::BitFlags> flagCommon;
  for (auto phr : phrase->subphrases())
  {
    if (!phr)
    {
      continue;
    }

    std::string rsrcType;
    smtk::model::BitFlags entry = 0;
    auto rsrc = phr->relatedResource();
    if (rsrc)
    {
      rsrcType = rsrc->typeName();
      auto comp = phr->relatedComponent();
      auto modelComp = std::dynamic_pointer_cast<smtk::model::Entity>(comp);
      //auto modelRsrc = std::dynamic_pointer_cast<smtk::model::Resource>(rsrc);
      if (modelComp)
      {
        entry = modelComp->entityFlags();
      }
    }
    if (entry && !rsrcType.empty())
    {
      auto it = flagCommon.find(rsrcType);
      if (it == flagCommon.end())
      {
        flagUnion[rsrcType] = entry;
        flagCommon[rsrcType] = entry;
      }
      else
      {
        flagUnion[rsrcType] |= entry;
        flagCommon[rsrcType] &= entry;
      }
    }
  }

  if (flagUnion.size() > 1)
  {
    message << sz << " items";
  }
  else
  {
    fCommon = flagCommon.begin()->second;
    fUnion = flagUnion.begin()->second;
    if (fCommon == fUnion)
    {
      message << sz << " " << smtk::model::Entity::flagSummary(fCommon, sz == 1 ? 0 : 1);
    }
    else
    {
      smtk::model::BitFlags etype = fCommon & smtk::model::ENTITY_MASK;
      smtk::model::BitFlags edims = fCommon & smtk::model::ANY_DIMENSION;
      bool pluralDims;
      std::string dimPhrase = smtk::model::Entity::flagDimensionList(
        edims ? edims : fUnion & smtk::model::ANY_DIMENSION, pluralDims);
      std::string name =
        etype ? smtk::model::Entity::flagSummary(etype, sz == 1 ? 0 : 1) : "entities";
      message << sz << " " << name << " of " << (pluralDims ? "dimensions" : "dimension") << " "
              << dimPhrase;
    }
  }
  return message.str();
}

} // view namespace
} // smtk namespace
