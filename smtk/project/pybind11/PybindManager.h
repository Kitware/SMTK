//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_project_Manager_h
#define pybind_smtk_project_Manager_h

#include <pybind11/pybind11.h>

#include "smtk/project/Manager.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/operation/Manager.h"
#include "smtk/project/Container.h"
#include "smtk/project/MetadataContainer.h"
#include "smtk/project/Project.h"
#include "smtk/resource/Manager.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::project::Manager > pybind11_init_smtk_project_Manager(py::module &m)
{
  PySharedPtrClass< smtk::project::Manager > instance(m, "Manager");
  instance
    .def("add", (bool (smtk::project::Manager::*)(::smtk::project::Project::Index const &, ::smtk::project::ProjectPtr const &)) &smtk::project::Manager::add, py::arg("arg0"), py::arg("arg1"))
    .def("add", (bool (smtk::project::Manager::*)(::smtk::project::ProjectPtr const &)) &smtk::project::Manager::add, py::arg("arg0"))
    .def_static("create", (std::shared_ptr<smtk::project::Manager> (*)(::smtk::resource::ManagerPtr const &, ::smtk::operation::ManagerPtr const &)) &smtk::project::Manager::create, py::arg("resourceManager"), py::arg("operationManager"))
    .def("createProject", [](smtk::project::Manager& manager, const std::string& projectType)
      {
        std::shared_ptr<smtk::common::Managers> empty;
        return manager.create(projectType, empty);
      }, py::arg("projectType")
    )
    .def("createProject", [](smtk::project::Manager& manager, const smtk::project::Project::Index& projectTypeIndex)
      {
        std::shared_ptr<smtk::common::Managers> empty;
        return manager.create(projectTypeIndex, empty);
      }, py::arg("projectTypeIndex")
    )
    .def("createProject", [](smtk::project::Manager& manager, const std::string& projectType, const smtk::common::UUID& uid)
      {
        std::shared_ptr<smtk::common::Managers> empty;
        return manager.create(projectType, uid, empty);
      }, py::arg("projectType"), py::arg("uuid")
    )
    .def("createProject", [](smtk::project::Manager& manager, const smtk::project::Project::Index& projectTypeIndex, const smtk::common::UUID& uid)
      {
        std::shared_ptr<smtk::common::Managers> empty;
        return manager.create(projectTypeIndex, uid, empty);
      }, py::arg("projectTypeIndex"), py::arg("uuid")
    )
    .def("find", (std::set<std::shared_ptr<smtk::project::Project>, std::less<std::shared_ptr<smtk::project::Project> >, std::allocator<std::shared_ptr<smtk::project::Project> > > (smtk::project::Manager::*)(::std::string const &)) &smtk::project::Manager::find, py::arg("arg0"))
    .def("find", (std::set<std::shared_ptr<smtk::project::Project>, std::less<std::shared_ptr<smtk::project::Project> >, std::allocator<std::shared_ptr<smtk::project::Project> > > (smtk::project::Manager::*)(::smtk::project::Project::Index const &)) &smtk::project::Manager::find, py::arg("arg0"))
    .def("get", (smtk::project::ProjectPtr (smtk::project::Manager::*)(::smtk::common::UUID const &)) &smtk::project::Manager::get, py::arg("id"))
    .def("get", (smtk::project::ConstProjectPtr (smtk::project::Manager::*)(::smtk::common::UUID const &) const) &smtk::project::Manager::get, py::arg("id"))
    .def("get", (smtk::project::ProjectPtr (smtk::project::Manager::*)(::std::string const &)) &smtk::project::Manager::get, py::arg("arg0"))
    .def("get", (smtk::project::ConstProjectPtr (smtk::project::Manager::*)(::std::string const &) const) &smtk::project::Manager::get, py::arg("arg0"))
    .def("metadata", &smtk::project::Manager::metadata)
    .def("metadataObservers", (smtk::project::Metadata::Observers & (smtk::project::Manager::*)()) &smtk::project::Manager::metadataObservers)
    .def("metadataObservers", (smtk::project::Metadata::Observers const & (smtk::project::Manager::*)() const) &smtk::project::Manager::metadataObservers)
    .def("observers", (smtk::project::Observers & (smtk::project::Manager::*)()) &smtk::project::Manager::observers)
    .def("observers", (smtk::project::Observers const & (smtk::project::Manager::*)() const) &smtk::project::Manager::observers)
    .def("operationManager", &smtk::project::Manager::operationManager)
    .def("projects", (smtk::project::Container & (smtk::project::Manager::*)()) &smtk::project::Manager::projects)
    .def("projects", (smtk::project::Container const & (smtk::project::Manager::*)() const) &smtk::project::Manager::projects)
    .def("registerProject", (bool (smtk::project::Manager::*)(const std::string&, const std::set<std::string>&,
    const std::set<std::string>&, const std::string&)) &smtk::project::Manager::registerProject, py::arg("name"), py::arg("resources"), py::arg("operations"), py::arg("version"))
    .def("registered", (bool (smtk::project::Manager::*)(::std::string const &) const) &smtk::project::Manager::registered, py::arg("arg0"))
    .def("registered", (bool (smtk::project::Manager::*)(::smtk::project::Project::Index const &) const) &smtk::project::Manager::registered, py::arg("arg0"))
    .def("remove", &smtk::project::Manager::remove, py::arg("arg0"))
    .def("resourceManager", &smtk::project::Manager::resourceManager)
    .def("unregisterOperation", (bool (smtk::project::Manager::*)(::std::string const &)) &smtk::project::Manager::unregisterOperation, py::arg("arg0"))
    .def("unregisterOperation", (bool (smtk::project::Manager::*)(::smtk::operation::Operation::Index const &)) &smtk::project::Manager::unregisterOperation, py::arg("arg0"))
    .def("unregisterProject", (bool (smtk::project::Manager::*)(::std::string const &)) &smtk::project::Manager::unregisterProject, py::arg("arg0"))
    .def("unregisterProject", (bool (smtk::project::Manager::*)(::smtk::project::Project::Index const &)) &smtk::project::Manager::unregisterProject, py::arg("arg0"))
    ;
  return instance;
}

#endif
