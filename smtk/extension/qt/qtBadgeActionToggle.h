//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtBadgeActionToggle_h
#define smtk_extension_qtBadgeActionToggle_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include "smtk/view/Badge.h"

#include <QItemSelection>

namespace smtk
{
namespace extension
{

/**\brief A bulk-capable toggle action for badges.
  *
  * This class inherits BadgeActionToggle to add an implementation
  * of visitRelatedPhrases() that will traverse the selected model
  * indices passed by the constructor.
  */
class SMTKQTEXT_EXPORT qtBadgeActionToggle : public smtk::view::BadgeActionToggle
{
public:
  qtBadgeActionToggle(QItemSelection& phrases);
  virtual ~qtBadgeActionToggle() = default;

  void visitRelatedPhrases(PhraseVisitor visitor) const override;

protected:
  qtBadgeActionToggle();

  QItemSelection& m_phrases;
};
} // namespace extension
} // namespace smtk

#endif
