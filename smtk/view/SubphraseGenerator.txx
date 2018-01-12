//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_SubphraseGenerator_txx
#define smtk_view_SubphraseGenerator_txx

#include "smtk/view/SubphraseGenerator.h"

#include "smtk/view/ComponentPhraseContent.h"
#include "smtk/view/PhraseListContent.h"
#include "smtk/view/ResourcePhraseContent.h"

namespace smtk
{
namespace view
{

template <typename T>
void SubphraseGenerator::addModelEntityPhrases(
  const T& ents, DescriptivePhrase::Ptr parent, int limit, DescriptivePhrases& result)
{
  if (limit < 0 || static_cast<int>(ents.size()) < limit)
  {
    for (typename T::const_iterator it = ents.begin(); it != ents.end(); ++it)
    {
      result.push_back(ComponentPhraseContent::createPhrase(it->component(), 0, parent));
    }
  }
  else
  {
    auto listEntry = DescriptivePhrase::create();
    DescriptivePhrases phrases;
    for (typename T::const_iterator it = ents.begin(); it != ents.end(); ++it)
    {
      phrases.push_back(ComponentPhraseContent::createPhrase(it->component(), 0, listEntry));
    }
    result.push_back(listEntry->setup(DescriptivePhraseType::COMPONENT_LIST, parent));
    auto content = PhraseListContent::create()->setup(parent, 0);
    listEntry->setContent(content);
    this->decoratePhrases(phrases);
    listEntry->manuallySetSubphrases(phrases, /* notify model: */ false);
  }
}

} // namespace view
} // namespace smtk

#endif
