//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_ViewWidgetFactory_h
#define smtk_view_ViewWidgetFactory_h

#include "smtk/CoreExports.h"

#include "smtk/common/Factory.h"

#include "smtk/view/BaseView.h"
#include "smtk/view/Information.h"

#include <map>
#include <string>

namespace smtk
{
namespace view
{
/**\brief A factory to create view widgets which is held by view managers
*/
class SMTKCORE_EXPORT ViewWidgetFactory
  : public smtk::common::Factory<BaseView, const smtk::view::Information&>
{
public:
  template <typename Type>
  void addAlias(const std::string& alias)
  {
    addAlias(smtk::view::typeIndex<Type>(), alias);
  }

  /// Add an alternative constructor name for a view widget.
  void addAlias(const std::size_t& typeIndex, const std::string& alias)
  {
    m_aliases[alias] = typeIndex;
  }

  /// Determine whether or not a Type is available using its alias.
  bool containsAlias(const std::string& alias) const
  {
    return m_aliases.find(alias) != m_aliases.end();
  }

  /// Create an instance of a Type using its type name.
  template <typename... Args>
  std::unique_ptr<BaseView> createFromAlias(const std::string& alias, Args&&... args) const
  {
    auto found = m_aliases.find(alias);
    if (found != m_aliases.end())
    {
      return createFromIndex(found->second, std::forward<Args>(args)...);
    }
    return std::unique_ptr<BaseView>();
  }

private:
  std::map<std::string, std::size_t> m_aliases;
};
}
}

#endif
