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
/// An icon factory creates icons for Resources and Components. An "Icon" in
/// this context is a binary array of the contents of an icon representation
/// given an input line and fill color. Currently, this corresponds to an ASCII
/// description of an SVG image.
class SMTKCORE_EXPORT IconFactory
{
public:
  using IconConstructor = std::function<std::string(const std::string&, const std::string&)>;

  /// Register an icon constructor identified by the resource or component it
  /// represents.
  template <typename ComponentType>
  bool registerIconConstructor(IconConstructor&&);

  /// Register an icon constructor identified by the type name of the resource
  /// or component it represents.
  bool registerIconConstructor(const std::string&, IconConstructor&&);

  /// Register a default icon constructor. This constructor is used if no
  /// constructor can be identified as associated with an input resource or
  /// component.
  bool registerDefaultIconConstructor(IconConstructor&&);

  /// Unregister an icon identified by the resource or component it represents.
  template <typename ComponentType>
  bool unregisterIconConstructor();

  /// Unregister an icon identified by the type name of the resource or component
  /// it represents.
  bool unregisterIconConstructor(const std::string&);

  /// Construct an icon identified by the type name of the resource or component
  /// it represents and the line and fill colors.
  std::string createIcon(
    const std::string& typeName, const std::string& lineColor, const std::string& fillColor) const;

  /// Construct an icon identified by the resource or component it represents
  /// and the line and fill colors.
  template <typename ComponentType>
  std::string createIcon(const std::string& lineColor, const std::string& fillColor) const;

  /// construct an icon idenfified by the resource or component it represents.
  std::string createIcon(
    const smtk::resource::PersistentObject&, const std::string& lineColor) const;

private:
  /// A container for all registered icon constructors.
  std::map<std::string, IconConstructor> m_iconConstructors;
};

template <typename ComponentType>
bool IconFactory::registerIconConstructor(IconConstructor&& iconConstructor)
{
  return registerIconConstructor(
    smtk::common::typeName<ComponentType>(), std::forward<IconConstructor>(iconConstructor));
}

template <typename ComponentType>
bool IconFactory::unregisterIconConstructor()
{
  return unregisterIconConstructor(smtk::common::typeName<ComponentType>());
}

template <typename ComponentType>
std::string IconFactory::createIcon(
  const std::string& lineColor, const std::string& fillColor) const
{
  return createIcon(smtk::common::typeName<ComponentType>(), lineColor, fillColor);
}
}
}

#endif
