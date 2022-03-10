//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_PyResource_h
#define smtk_resource_PyResource_h

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <type_traits>

#include "smtk/io/Logger.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

#include "smtk/common/PythonInterpreter.h"

namespace pybind11
{
namespace detail
{

template <>
struct type_caster<std::shared_ptr<smtk::resource::Resource> >
{
  PYBIND11_TYPE_CASTER(std::shared_ptr<smtk::resource::Resource>, _("Resource"));

  using ResourceCaster =
    copyable_holder_caster<smtk::resource::Resource, std::shared_ptr<smtk::resource::Resource> >;

  bool load(handle src, bool b)
  {
    ResourceCaster oc;
    bool success = oc.load(src, b);
    if (!success)
    {
      return false;
    }

    auto py_obj = reinterpret_borrow<object>(src);
    auto base_ptr = static_cast<std::shared_ptr<smtk::resource::Resource> >(oc);

    // Construct a shared_ptr to the object
    auto py_obj_ptr = std::shared_ptr<object>{ new object{ py_obj },
      [](object* py_object_ptr) {
        // It's possible that when the shared_ptr dies we won't have the
        // gil (if the last holder is in a non-Python thread), so we
        // make sure to acquire it in the deleter.
        gil_scoped_acquire gil;
        delete py_object_ptr;
      } };

    value = std::shared_ptr<smtk::resource::Resource>(py_obj_ptr, base_ptr.get());
    return true;
  }

  static handle cast(
    std::shared_ptr<smtk::resource::Resource> base, return_value_policy rvp, handle h)
  {
    return ResourceCaster::cast(base, rvp, h);
  }
};

template <>
struct is_holder_type<smtk::resource::Resource, std::shared_ptr<smtk::resource::Resource> >
  : std::true_type
{
};

template <>
struct type_caster<std::shared_ptr<smtk::resource::Component> >
{
  PYBIND11_TYPE_CASTER(std::shared_ptr<smtk::resource::Component>, _("Component"));

  using ComponentCaster =
    copyable_holder_caster<smtk::resource::Component, std::shared_ptr<smtk::resource::Component> >;

  bool load(handle src, bool b)
  {
    ComponentCaster oc;
    bool success = oc.load(src, b);
    if (!success)
    {
      return false;
    }

    auto py_obj = reinterpret_borrow<object>(src);
    auto base_ptr = static_cast<std::shared_ptr<smtk::resource::Component> >(oc);

    // Construct a shared_ptr to the object
    auto py_obj_ptr = std::shared_ptr<object>{ new object{ py_obj },
      [](object* py_object_ptr) {
        // It's possible that when the shared_ptr dies we won't have the
        // gil (if the last holder is in a non-Python thread), so we
        // make sure to acquire it in the deleter.
        gil_scoped_acquire gil;
        delete py_object_ptr;
      } };

    value = std::shared_ptr<smtk::resource::Component>(py_obj_ptr, base_ptr.get());
    return true;
  }

  static handle cast(
    std::shared_ptr<smtk::resource::Component> base, return_value_policy rvp, handle h)
  {
    return ComponentCaster::cast(base, rvp, h);
  }
};

template <>
struct is_holder_type<smtk::resource::Component, std::shared_ptr<smtk::resource::Component> >
  : std::true_type
{
};
}
}

namespace smtk
{
namespace resource
{
class PyResource : public Resource
{
public:
  PyResource() = default;

  ~PyResource() override = default;

  static std::shared_ptr<smtk::resource::Resource> create(
    std::string modulename, std::string className, smtk::resource::Resource::Index index)
  {
    // Import the module containing our resource
    pybind11::module module = pybind11::module::import(modulename.c_str());

    // Create an instance of our resource
    pybind11::object obj = module.attr(className.c_str())();

    // For C++ resources, index() is a compile-time intrinsic of the
    // resource class. Python resources only come into existence at runtime,
    // though, so we need to manually set a python resource's index.
    obj.cast<std::shared_ptr<smtk::resource::PyResource> >()->setIndex(index);

    // The precedent for python resource names is established to be the
    // modulename.className. We follow that convention here.
    obj.cast<std::shared_ptr<smtk::resource::PyResource> >()->setTypeName(
      modulename + "." + className);

    return obj.cast<std::shared_ptr<smtk::resource::Resource> >();
  }

  std::string typeName() const override { return m_typeName; }
  Index index() const override { return m_index; }

  const smtk::common::UUID& id() const override
  {
    PYBIND11_OVERLOAD(const smtk::common::UUID&, Resource, id, );
  }

  bool setId(const smtk::common::UUID& newId) override
  {
    PYBIND11_OVERLOAD(bool, Resource, setId, newId);
  }

  std::string name() const override { PYBIND11_OVERLOAD(std::string, Resource, name, ); }

  bool isOfType(const std::string& typeName) const override { return m_typeName == typeName; }
  int numberOfGenerationsFromBase(const std::string& typeName) const override
  {
    return (this->isOfType(typeName) ? 0 : 1 + Resource::numberOfGenerationsFromBase(typeName));
  }

  ComponentPtr find(const smtk::common::UUID& compId) const override
  {
    PYBIND11_OVERLOAD_PURE(ComponentPtr, Resource, find, compId);
  }

  std::function<bool(const Component&)> queryOperation(
    const std::string& queryString) const override
  {
    PYBIND11_OVERLOAD_PURE(
      std::function<bool(const Component&)>, Resource, queryOperation, queryString);
  }

  void visit(std::function<void(const Component::Ptr&)>& v) const override
  {
    PYBIND11_OVERLOAD_PURE(void, Resource, visit, v);
  }

private:
  void setIndex(Index index) { m_index = index; }
  void setTypeName(const std::string& typeName) { m_typeName = typeName; }

  Index m_index;
  std::string m_typeName;
};
}
}

#endif
