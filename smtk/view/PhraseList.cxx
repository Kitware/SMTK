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
  message << sz << " items";

  m_title = message.str();
}

} // view namespace
} // smtk namespace
