//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_qt_TypeAndColorBadge_h
#define __smtk_extension_qt_TypeAndColorBadge_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/qt/Exports.h"

#include "smtk/view/ObjectIconBadge.h"

namespace smtk
{
namespace extension
{
namespace qt
{

/**\brief A badge that illustrates the type and set the color of a persistent object.
  *
  */
class SMTKQTEXT_EXPORT TypeAndColorBadge : public smtk::view::ObjectIconBadge
{
public:
  smtkTypeMacro(smtk::extension::qt::TypeAndColorBadge);
  smtkSuperclassMacro(smtk::view::ObjectIconBadge);
  using DescriptivePhrase = smtk::view::DescriptivePhrase;
  using BadgeAction = smtk::view::BadgeAction;

  TypeAndColorBadge();
  TypeAndColorBadge(smtk::view::BadgeSet&, const smtk::view::Configuration::Component&);
  virtual ~TypeAndColorBadge();

  bool action(const DescriptivePhrase* phrase, const BadgeAction& act) override;
};
} // namespace qt
} // namespace extension
} // namespace smtk

#endif
