//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtBadgeActionToggle.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"

#include <QItemSelection>

namespace // anonymous
{
QItemSelection s_dummy;
}

namespace smtk
{
namespace extension
{

qtBadgeActionToggle::qtBadgeActionToggle(QItemSelection& phrases)
  : m_phrases(phrases)
{
}

qtBadgeActionToggle::qtBadgeActionToggle()
  : m_phrases(s_dummy)
{
}

void qtBadgeActionToggle::visitRelatedPhrases(PhraseVisitor visitor) const
{
  for (auto& index : m_phrases.indexes())
  {
    auto phrase =
      index.data(qtDescriptivePhraseModel::PhrasePtrRole).value<smtk::view::DescriptivePhrasePtr>();
    if (phrase)
    {
      bool shouldStop = visitor(phrase.get());
      if (shouldStop)
      {
        break;
      }
    }
  }
}
} // namespace extension
} // namespace smtk
