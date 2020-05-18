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

#include "smtk/model/EntityRef.h"

#include "smtk/view/ComponentPhraseContent.h"
#include "smtk/view/PhraseListContent.h"
#include "smtk/view/ResourcePhraseContent.h"

#include <algorithm>

namespace smtk
{
namespace view
{
template <typename T>
void SubphraseGenerator::PreparePath(T& result, const T& parentPath, int childIndex)
{
  result.reserve(parentPath.size() + 1);
  for (auto idx : parentPath)
  {
    result.push_back(idx);
  }
  result.push_back(childIndex);
}

template <typename T>
int SubphraseGenerator::IndexFromTitle(const std::string& title, const T& phrases)
{
  // TODO: Use bisection to speed this up.
  int ii = 0;
  for (auto phrase : phrases)
  {
    if (title < phrase->title())
    {
      return ii;
    }
    ++ii;
  }
  return ii;
}

// template <typename T>
// PhraseListContentPtr SubphraseGenerator::addComponentPhrases(const T& components,
//   DescriptivePhrase::Ptr parent, DescriptivePhrases& result, int mutability, bool decorate,
//   std::function<bool(const DescriptivePhrasePtr&, const DescriptivePhrasePtr&)> comparator)
// {
//   auto listEntry = DescriptivePhrase::create();
//   DescriptivePhrases phrases;
//   for (typename T::const_iterator it = components.begin(); it != components.end(); ++it)
//   {
//     phrases.push_back(ComponentPhraseContent::createPhrase(*it, mutability, listEntry));
//   }
//   if (comparator)
//   {
//     std::sort(phrases.begin(), phrases.end(), comparator);
//   }
//   result.push_back(listEntry->setup(DescriptivePhraseType::COMPONENT_LIST, parent));
//   auto content = PhraseListContent::create()->setup(parent, -1, -1, 0);
//   listEntry->setContent(content);
//   if (decorate)
//   {
//     this->decoratePhrases(phrases);
//   }
//   listEntry->manuallySetSubphrases(phrases, /* notify model: */ false);
//   return content;
// }

template <typename T>
void SubphraseGenerator::filterModelEntityPhraseCandidates(T& ents)
{
  T filteredEntities;
  filteredEntities.reserve(ents.size());
  for (const auto& ent : ents)
  {
    bool isHidden = ent.exclusions(smtk::model::Exclusions::ViewPresentation) ? true : false;
    if (!isHidden)
    {
      filteredEntities.push_back(ent);
    }
  }
  filteredEntities.swap(ents);
}

template <typename T>
PhraseListContentPtr SubphraseGenerator::addModelEntityPhrases(const T& ents,
  DescriptivePhrase::Ptr parent, int limit, DescriptivePhrases& result, int mutability,
  bool /*decorate*/,
  std::function<bool(const DescriptivePhrasePtr&, const DescriptivePhrasePtr&)> comparator)
{
  if (limit < 0 || static_cast<int>(ents.size()) < limit)
  {
    for (typename T::const_iterator it = ents.begin(); it != ents.end(); ++it)
    {
      bool isHidden = it->exclusions(smtk::model::Exclusions::ViewPresentation) ? true : false;
      if (isHidden)
      {
        continue;
      }
      result.push_back(ComponentPhraseContent::createPhrase(it->component(), mutability, parent));
    }
    if (comparator)
    {
      std::sort(result.begin(), result.end(), comparator);
    }
  }
  else
  {
    auto listEntry = DescriptivePhrase::create();
    DescriptivePhrases phrases;
    for (typename T::const_iterator it = ents.begin(); it != ents.end(); ++it)
    {
      bool isHidden = it->exclusions(smtk::model::Exclusions::ViewPresentation) ? true : false;
      if (isHidden)
      {
        continue;
      }
      phrases.push_back(
        ComponentPhraseContent::createPhrase(it->component(), mutability, listEntry));
    }
    if (comparator)
    {
      std::sort(phrases.begin(), phrases.end(), comparator);
    }
    result.push_back(listEntry->setup(DescriptivePhraseType::COMPONENT_LIST, parent));
    auto content = PhraseListContent::create()->setup(parent, -1, -1, 0);
    listEntry->setContent(content);
    listEntry->manuallySetSubphrases(phrases, /* notify model: */ false);
    return content;
  }
  return nullptr;
}

} // namespace view
} // namespace smtk

#endif
