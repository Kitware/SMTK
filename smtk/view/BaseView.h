//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_BaseView_h
#define smtk_view_BaseView_h

#include "smtk/CoreExports.h"

#include "smtk/SharedFromThis.h"

#include <typeindex>

namespace smtk
{
namespace view
{
class SMTKCORE_EXPORT BaseView
{
public:
  smtkTypenameMacroBase(BaseView);

  virtual ~BaseView() = 0;

  /// index is a compile-time intrinsic of the derived view; as such, it
  /// cannot be set.
  virtual std::size_t index() const { return std::type_index(typeid(*this)).hash_code(); }
};

template<typename ViewWidgetType>
std::size_t typeIndex()
{
  return std::type_index(typeid(ViewWidgetType)).hash_code();
}
} // namespace view
} // namespace smtk

#endif
