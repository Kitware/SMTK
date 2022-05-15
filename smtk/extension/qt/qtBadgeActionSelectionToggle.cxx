//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtBadgeActionSelectionToggle.h"

#include "smtk/view/PhraseModel.h"

namespace smtk
{
namespace extension
{

qtBadgeActionSelectionToggle::qtBadgeActionSelectionToggle(
  std::shared_ptr<smtk::view::PhraseModel>& model,
  std::shared_ptr<smtk::view::Selection>& seln,
  const std::string& selnLabel,
  bool exactMatch)
  : m_model(model)
  , m_selection(seln)
  , m_selectionLabel(selnLabel)
  , m_exactMatch(exactMatch)
{
}

void qtBadgeActionSelectionToggle::visitRelatedPhrases(PhraseVisitor visitor) const
{
  if (!m_model || !m_selection)
  {
    return;
  }
  auto items =
    m_selection->currentSelectionByValueAs<std::set<smtk::resource::PersistentObject::Ptr>>(
      m_selectionLabel, m_exactMatch);
  const auto& objectIdMap = m_model->uuidPhraseMap();
  for (const auto& item : items)
  {
    auto it = objectIdMap.find(item->id());
    if (it != objectIdMap.end())
    {
      // Iterate over all phrases that refer to this item as the subject.
      // It may be sufficient to only visit a single phrase, but there is
      // no guarantee that all the phrases are of the same type.
      for (const auto& weakPhrase : it->second)
      {
        if (auto phrase = weakPhrase.lock())
        {
          bool shouldStop = visitor(phrase.get());
          if (shouldStop)
          {
            return;
          }
        }
      }
    }
  }
}
} // namespace extension
} // namespace smtk
