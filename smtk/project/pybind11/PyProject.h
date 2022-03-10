//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_PyProject_h
#define smtk_project_PyProject_h

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <type_traits>

#include "smtk/io/Logger.h"

#include "smtk/project/Project.h"

#include "smtk/common/PythonInterpreter.h"

namespace pybind11
{
namespace detail
{

template <>
struct type_caster<std::shared_ptr<smtk::project::Project> >
{
  PYBIND11_TYPE_CASTER(std::shared_ptr<smtk::project::Project>, _("Project"));

  using ProjectCaster =
    copyable_holder_caster<smtk::project::Project, std::shared_ptr<smtk::project::Project> >;

  bool load(handle src, bool b)
  {
    ProjectCaster oc;
    bool success = oc.load(src, b);
    if (!success)
    {
      return false;
    }

    auto py_obj = reinterpret_borrow<object>(src);
    auto base_ptr = static_cast<std::shared_ptr<smtk::project::Project> >(oc);

    // Construct a shared_ptr to the object
    auto py_obj_ptr = std::shared_ptr<object>{ new object{ py_obj },
      [](object* py_object_ptr) {
        // It's possible that when the shared_ptr dies we won't have the
        // gil (if the last holder is in a non-Python thread), so we
        // make sure to acquire it in the deleter.
        gil_scoped_acquire gil;
        delete py_object_ptr;
      } };

    value = std::shared_ptr<smtk::project::Project>(py_obj_ptr, base_ptr.get());
    return true;
  }

  static handle cast(
    std::shared_ptr<smtk::project::Project> base, return_value_policy rvp, handle h)
  {
    return ProjectCaster::cast(base, rvp, h);
  }
};

template <>
struct is_holder_type<smtk::project::Project, std::shared_ptr<smtk::project::Project> >
  : std::true_type
{
};
}
}

namespace smtk
{
namespace project
{
class PyProject : public Project
{
public:
  PyProject() = default;

  ~PyProject() override = default;

  static std::shared_ptr<smtk::project::Project> create(
    std::string modulename,
    std::string className,
    smtk::project::Project::Index index,
    const smtk::common::UUID& uuid = smtk::common::UUID::null(),
    const std::shared_ptr<smtk::common::Managers>& managers = nullptr
    )
  {
    // Import the module containing our project
    pybind11::module module = pybind11::module::import(modulename.c_str());

    // Create an instance of our project
    pybind11::object obj = module.attr(className.c_str())();

    // For C++ projects, index() is a compile-time intrinsic of the
    // project class. Python projects only come into existence at runtime,
    // though, so we need to manually set a python project's index.
    obj.cast<std::shared_ptr<smtk::project::PyProject> >()->setIndex(index);

    // The precedent for python project names is estabilished to be the
    // modulename.className. We follow that convention here.
    obj.cast<std::shared_ptr<smtk::project::PyProject> >()->setTypeName(
      modulename + "." + className);

    if (!uuid.isNull())
    {
      obj.cast<std::shared_ptr<smtk::project::PyProject> >()->setId(uuid);
    }
    (void) managers;
    return obj.cast<std::shared_ptr<smtk::project::Project> >();
  }

  std::string typeName() const override { return m_typeName; }
  Index index() const override { return m_index; }

  const smtk::common::UUID& id() const override
  {
    PYBIND11_OVERLOAD(const smtk::common::UUID&, Project, id, );
  }

  bool setId(const smtk::common::UUID& newId) override
  {
    PYBIND11_OVERLOAD(bool, Project, setId, newId);
  }

  std::string name() const override { PYBIND11_OVERLOAD(std::string, Project, name, ); }

  bool isOfType(const std::string& typeName) const override { return m_typeName == typeName; }
  int numberOfGenerationsFromBase(const std::string& typeName) const override
  {
    return (this->isOfType(typeName) ? 0 : 1 + Project::numberOfGenerationsFromBase(typeName));
  }

  smtk::resource::ComponentPtr find(const smtk::common::UUID& compId) const override
  {
    PYBIND11_OVERLOAD(smtk::resource::ComponentPtr, Project, find, compId);
  }

  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string& queryString) const override
  {
    PYBIND11_OVERLOAD(
      std::function<bool(const smtk::resource::Component&)>, Project, queryOperation, queryString);
  }

  void visit(std::function<void(const smtk::resource::Component::Ptr&)>& v) const override
  {
    PYBIND11_OVERLOAD(void, Project, visit, v);
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
