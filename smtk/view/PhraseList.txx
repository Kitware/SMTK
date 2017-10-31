//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_PhraseList_txx
#define smtk_view_PhraseList_txx

#include "smtk/view/PhraseList.h"

#include <algorithm>

namespace smtk
{
namespace view
{

template <typename T>
PhraseList::Ptr PhraseList::setup(
  const T& phrases, DescriptivePhrase::Ptr parnt, DescriptivePhraseType listType)
{
  this->DescriptivePhrase::setup(listType, parnt);
  m_subphrases.clear();
  m_commonFlags = smtk::model::INVALID;
  m_unionFlags = 0;
  for (typename T::const_iterator it = phrases.begin(); it != phrases.end(); ++it)
  {
    DescriptivePhrase::Ptr entry = dynamic_pointer_cast<DescriptivePhrase>(*it);
    // Change the parent of entry to this phrase:
    entry->setup(entry->phraseType(), shared_from_this());
    m_subphrases.push_back(entry);
    // If the entry is a model component, update the entity-type flags describing all entries:
    smtk::resource::ComponentPtr comp = entry->relatedComponent();
    if (comp)
    {
      auto modelEnt = dynamic_pointer_cast<smtk::model::Entity>(comp);
      if (modelEnt)
      {
        m_commonFlags &= modelEnt->entityFlags();
        m_unionFlags |= modelEnt->entityFlags();
      }
    }
  }
  std::sort(m_subphrases.begin(), m_subphrases.end(), DescriptivePhrase::compareByTypeThenTitle);
  m_subphrasesBuilt = true;
  return shared_from_this();
}

} // view namespace
} // smtk namespace

#endif
