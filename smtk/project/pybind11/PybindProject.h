//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_project_Project_h
#define pybind_smtk_project_Project_h

#include <pybind11/pybind11.h>

#include "smtk/project/Project.h"

#include "smtk/project/pybind11/PyProject.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/project/Manager.h"
#include "smtk/project/OperationFactory.h"
#include "smtk/project/ResourceContainer.h"
#include "smtk/resource/Component.h"
#include "smtk/task/Manager.h"

namespace py = pybind11;

inline PySharedPtrClass<smtk::project::Project> pybind11_init_smtk_project_Project(py::module& m)
{
  PySharedPtrClass<smtk::project::Project, smtk::project::PyProject, smtk::resource::Resource>
    instance(m, "Project");
  instance
    .def(py::init_alias<>())
    .def_static("create", &smtk::project::Project::create, py::arg("typeName") = "")
    .def("find", &smtk::project::Project::find, py::arg("id"))
    .def("index", &smtk::project::Project::index)
    .def("manager", &smtk::project::Project::manager)
    .def("name", &smtk::project::Project::name)
    .def("operations", (smtk::project::OperationFactory const& (smtk::project::Project::*)() const) &smtk::project::Project::operations)
    .def("operations", (smtk::project::OperationFactory & (smtk::project::Project::*)()) &
        smtk::project::Project::operations)
    .def("queryOperation", &smtk::project::Project::queryOperation, py::arg("queryString"))
    .def("resources", (smtk::project::ResourceContainer const& (smtk::project::Project::*)() const) &smtk::project::Project::resources,
        py::return_value_policy::reference_internal)
    .def("resources", (smtk::project::ResourceContainer & (smtk::project::Project::*)()) &smtk::project::Project::resources,
        py::return_value_policy::reference_internal)
    .def("setId", &smtk::project::Project::setId, py::arg("newId"))
    .def("setVersion", &smtk::project::Project::setVersion, py::arg("version"))
    .def("shared_from_this",
      (std::shared_ptr<const smtk::project::Project> (smtk::project::Project::*)() const) &smtk::project::Project::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::project::Project> (smtk::project::Project::*)()) &smtk::project::Project::shared_from_this)
    .def("typeName", &smtk::project::Project::typeName)
    .def("version", &smtk::project::Project::version)
    .def("taskManager", [](const smtk::project::Project::Ptr& project) { return &project->taskManager(); })
    .def("visit", &smtk::project::Project::visit, py::arg("visitor"));
  return instance;
}

#endif
