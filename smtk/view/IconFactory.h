//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_IconFactory_h
#define smtk_view_IconFactory_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include "smtk/common/TypeName.h"

#include <functional>
#include <map>
#include <string>
#include <utility>

namespace smtk
{
namespace resource
{
class PersistentObject;
}
namespace view
{
/// An icon factory creates icons for Resources and their Components. An "Icon"
/// in this context is a binary array of the contents of an icon representation
/// given an input secondary color (a common color offset from the background
/// used to draw unhighlighted icon features; e.g. black for a white
/// background). Currently, this corresponds to an ASCII description of an SVG
/// image.
///
/// An IconConstructor is a functor that accepts a PersistentObject and a string
/// representing a secondary color and returns an icon for the PersistentObject.
/// Because Resources contain hierarchical queries (e.g. is Resource A derived
/// from Resource B), IconConstructors registered to a Resource are expected to
/// return icons for the Resource and its Components, facilitating the
/// inheritance of icon sets from parent Resources. Icon constructors can be
/// defined for any PersistentObject, however; if an IconConstructor is
/// registered to a PersistentObject type, this IconConstructor will have highest
/// priority when an icon for this PersistentObject is requested.
class SMTKCORE_EXPORT IconFactory
{
public:
  using IconConstructor =
    std::function<std::string(const smtk::resource::PersistentObject&, const std::string&)>;

  /// Register an icon constructor identified by the resource it represents.
  template <typename ResourceType>
  bool registerIconConstructor(IconConstructor&&);

  /// Register an icon constructor identified by the type name of the resource
  /// it represents.
  bool registerIconConstructor(const std::string&, IconConstructor&&);

  /// Register a default icon constructor. This constructor is used if no
  /// constructor can be identified as associated with an input resource.
  bool registerDefaultIconConstructor(IconConstructor&&);

  /// Unregister an icon identified by the resource it represents.
  template <typename ResourceType>
  bool unregisterIconConstructor();

  /// Unregister an icon identified by the type name of the resource it
  /// represents.
  bool unregisterIconConstructor(const std::string&);

  /// construct an icon idenfified by the resource or component it represents.
  std::string createIcon(
    const smtk::resource::PersistentObject&, const std::string& secondaryColor) const;

private:
  /// A container for all registered icon constructors.
  std::map<std::string, IconConstructor> m_iconConstructors;
};

template <typename ResourceType>
bool IconFactory::registerIconConstructor(IconConstructor&& iconConstructor)
{
  return registerIconConstructor(
    smtk::common::typeName<ResourceType>(), std::forward<IconConstructor>(iconConstructor));
}

template <typename ResourceType>
bool IconFactory::unregisterIconConstructor()
{
  return unregisterIconConstructor(smtk::common::typeName<ResourceType>());
}
}
}

#endif
