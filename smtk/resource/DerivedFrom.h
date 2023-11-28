//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_DerivedFrom_h
#define smtk_resource_DerivedFrom_h

#include "smtk/common/TypeName.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace resource
{

/// Derived resources are subclassed from smtk::resource::Resource. Queries
/// concerning the relationship between a derived resource and its parent
/// resources are made using the methods smtk::resource::Resource::isOfType().
/// To ensure that these methods reflect the proper inheritance of a Resource,
/// derived resources do not directly derive from smtk::resource::Resource;
/// instead, they derive from smtk::resource::DerivedFrom<Self, Parent>, where
/// Self is the class being defined and Parent is either
/// smtk::resource::Resource or one of its derived classes.
template<typename Self, typename Parent>
class DerivedFrom : public Parent
{
public:
  using ParentResource = Parent;
  using Superclass = Parent;

  // Provide a type-alias to the derived type so it can invoke our constructor.
  using DirectSuperclass = DerivedFrom<Self, Parent>;

  /// A static index for this resource type.
  ///
  /// NOTE: because we are using CRTP, it is possible to make this value static
  ///       and redefined for each resource type, regardless of inheritance.
  static const Resource::Index type_index;

  /// given a resource index, return whether or not this resource is or is
  /// derived from the resource described by the index.
  bool isOfType(const Resource::Index& index) const override
  {
    return DerivedFrom<Self, Parent>::type_index == index ? true : ParentResource::isOfType(index);
  }

  /// given a resource's unique name, return whether or not this resource is or
  /// is derived from the resource described by the name.
  bool isOfType(const std::string& typeName) const override
  {
    return smtk::common::typeName<Self>() == typeName ? true : ParentResource::isOfType(typeName);
  }

  int numberOfGenerationsFromBase(const std::string& typeName) const override
  {
    return (
      smtk::common::typeName<Self>() == typeName
        ? 0
        : 1 + ParentResource::numberOfGenerationsFromBase(typeName));
  }

  DerivedFrom(const DerivedFrom&) = delete;

protected:
  /// Forward all constructor arguments to the parent class.
  template<typename... T>
  DerivedFrom(T&&... all)
    : Parent(std::forward<T>(all)...)
  {
  }

  // NOLINTNEXTLINE(modernize-use-equals-delete)
  DerivedFrom(DerivedFrom&& rhs) noexcept = default;
};

template<typename Self, typename Parent>
const Resource::Index DerivedFrom<Self, Parent>::type_index =
  std::type_index(typeid(Self)).hash_code();
} // namespace resource
} // namespace smtk

#endif // smtk_resource_DerivedFrom_h
