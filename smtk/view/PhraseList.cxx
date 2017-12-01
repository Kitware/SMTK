//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/PhraseList.h"
#include "smtk/view/PhraseList.txx"

#include "smtk/model/EntityRef.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace view
{

PhraseList::PhraseList()
  : m_commonFlags(smtk::model::INVALID)
  , m_unionFlags(0)
{
  // only color is mutable
}

/// Show the entity name (or a default name) in the title
std::string PhraseList::title()
{
  if (m_title.empty())
  {
    this->generateTitle();
  }
  return m_title;
}

/// Show the entity type in the subtitle
std::string PhraseList::subtitle()
{
  return std::string();
}

/// The list of entities to be presented.
smtk::resource::ComponentArray PhraseList::relatedComponents() const
{
  smtk::resource::ComponentArray result;
  auto phrases = this->subphrases();
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
  * These are used by PhraseList to choose an appropriate summary
  * description of the entities.
  */
void PhraseList::setModelFlags(smtk::model::BitFlags commonFlags, smtk::model::BitFlags unionFlags)
{
  m_commonFlags = commonFlags;
  m_unionFlags = unionFlags;
}

/**\brief Return whether the related color is mutable or not.
  *
  * This will require special logic to match the behavior of EntityListPhrase.
  */
bool PhraseList::isRelatedColorMutable() const
{
  return false;
}

smtk::resource::FloatList PhraseList::relatedColor() const
{
  return smtk::resource::FloatList(4, -1.);
}

void PhraseList::generateTitle()
{
  // TODO: Handle homogenous and heterogenous subphrase lists
  //       the way its counterpart in smtk::model does.
  std::ostringstream message;
  smtk::resource::ComponentArray::size_type sz = this->subphrases().size();
  std::map<std::string, smtk::model::BitFlags> flagUnion;
  std::map<std::string, smtk::model::BitFlags> flagCommon;
  for (auto phr : this->subphrases())
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
      rsrcType = rsrc->uniqueName();
      auto comp = phr->relatedComponent();
      auto modelComp = std::dynamic_pointer_cast<smtk::model::Entity>(comp);
      //auto modelRsrc = std::dynamic_pointer_cast<smtk::model::Manager>(rsrc);
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
    smtk::model::BitFlags fCommon = flagCommon.begin()->second;
    smtk::model::BitFlags fUnion = flagUnion.begin()->second;
    if (fCommon == fUnion)
    {
      message << sz << " " << smtk::model::Entity::flagSummary(fCommon, sz == 1 ? 0 : 1);
    }
    else
    {
      smtk::model::BitFlags etype = this->m_commonFlags & smtk::model::ENTITY_MASK;
      smtk::model::BitFlags edims = this->m_commonFlags & smtk::model::ANY_DIMENSION;
      bool pluralDims;
      std::string dimPhrase = smtk::model::Entity::flagDimensionList(
        edims ? edims : this->m_unionFlags & smtk::model::ANY_DIMENSION, pluralDims);
      std::string name =
        etype ? smtk::model::Entity::flagSummary(etype, sz == 1 ? 0 : 1) : "entities";
      message << sz << " " << name << " of " << (pluralDims ? "dimensions" : "dimension")
              << dimPhrase;
    }
  }

  m_title = message.str();
}

} // view namespace
} // smtk namespace
