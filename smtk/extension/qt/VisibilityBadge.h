//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_VisibilityBadge_h
#define smtk_extension_qt_VisibilityBadge_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/qt/Exports.h"

#include "smtk/view/Badge.h"
#include "smtk/view/BadgeSet.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/DescriptivePhrase.h"

#include <QObject>

namespace smtk
{
namespace extension
{
namespace qt
{

/**\brief An empty base class for badges controlling visibility of objects
  *       (in other views).
  *
  * \sa smtk::extension::paraview::appcomponents::VisibilityBadge
  */
class SMTKQTEXT_EXPORT VisibilityBadge : public QObject, public smtk::view::Badge
{
  Q_OBJECT
public:
  VisibilityBadge();
  virtual ~VisibilityBadge();
};
}
}
}

#endif
