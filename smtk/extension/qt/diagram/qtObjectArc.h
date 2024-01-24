//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtObjectArc_h
#define smtk_extension_qtObjectArc_h

#include "smtk/extension/qt/diagram/qtBaseArc.h"

namespace smtk
{
namespace extension
{

/**\brief An arc between nodes in a diagram that represents an object.
  *
  * Usually, arcs are rendered between nodes representing persistent objects
  * but the arc itself does not represent a persistent object.
  * However, it is possible for an arc to also represent a persistent object.
  * If it does, it should inherit this class instead of qtBaseArc.
  */
class SMTKQTEXT_EXPORT qtObjectArc : public qtBaseArc
{
  Q_OBJECT

public:
  smtkSuperclassMacro(qtBaseArc);
  smtkTypeMacro(smtk::extension::qtObjectArc);

  template<typename... Args>
  qtObjectArc(Args&&... args)
    : Superclass(std::forward<Args>(args)...)
  {
  }

  /// Require subclasses to provide a persistent object.
  virtual smtk::resource::PersistentObject* object() const = 0;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtObjectArc_h
