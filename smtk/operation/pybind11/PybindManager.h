//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_Manager_h
#define pybind_smtk_operation_Manager_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/operation/Manager.h"

#include "smtk/operation/operators/ImportPythonOperation.h"
#include "smtk/operation/Metadata.h"
#include "smtk/operation/MetadataContainer.h"
#include "smtk/operation/Operation.h"

#include "smtk/operation/pybind11/PyOperation.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"

#include <vector>

namespace py = pybind11;

inline PySharedPtrClass< smtk::operation::Manager > pybind11_init_smtk_operation_Manager(py::module &m)
{
  PySharedPtrClass< smtk::operation::Manager > instance(m, "Manager");
  instance
    .def("availableOperations", (std::set<std::string> (smtk::operation::Manager::*)() const) &smtk::operation::Manager::availableOperations)
    .def("availableOperations", (std::set<smtk::operation::Operation::Index> (smtk::operation::Manager::*)(const smtk::resource::ComponentPtr&) const) &smtk::operation::Manager::availableOperations)
    .def_static("create", (std::shared_ptr<smtk::operation::Manager> (*)()) &smtk::operation::Manager::create)
    .def_static("create", (std::shared_ptr<smtk::operation::Manager> (*)(::std::shared_ptr<smtk::operation::Manager> &)) &smtk::operation::Manager::create, py::arg("ref"))
    .def("createOperation", (std::shared_ptr<smtk::operation::Operation> (smtk::operation::Manager::*)(::std::string const &)) &smtk::operation::Manager::create, py::arg("arg0"))
    .def("createOperation", (std::shared_ptr<smtk::operation::Operation> (smtk::operation::Manager::*)(::smtk::operation::Operation::Index const &)) &smtk::operation::Manager::create, py::arg("arg0"))
//    .def("metadata", [](smtk::operation::Manager& man) { std::vector<std::reference_wrapper<smtk::operation::Metadata>> vec; vec.reserve(man.metadata().size()); for (auto md : man.metadata()) { vec.push_back(md); } return vec; })
    .def("metadataObservers", (smtk::operation::Metadata::Observers & (smtk::operation::Manager::*)()) &smtk::operation::Manager::metadataObservers)
    .def("metadataObservers", (smtk::operation::Metadata::Observers const & (smtk::operation::Manager::*)() const) &smtk::operation::Manager::metadataObservers)
    .def("observers", (smtk::operation::Observers & (smtk::operation::Manager::*)()) &smtk::operation::Manager::observers, pybind11::return_value_policy::reference_internal)
    .def("observers", (smtk::operation::Observers const & (smtk::operation::Manager::*)() const) &smtk::operation::Manager::observers, pybind11::return_value_policy::reference_internal)
    .def("registered", (bool (smtk::operation::Manager::*)(const std::string&) const) &smtk::operation::Manager::registered, py::arg("typeName"))
    .def("registerResourceManager", &smtk::operation::Manager::registerResourceManager, py::arg("arg0"))
    .def("registerOperation", [](smtk::operation::Manager& manager, const std::string& moduleName, const std::string& opName){
        return smtk::operation::ImportPythonOperation::importOperation(manager, moduleName, opName);
      })
    .def("unregisterOperation", (bool (smtk::operation::Manager::*)(const std::string&)) &smtk::operation::Manager::unregisterOperation, py::arg("typeName"))
    .def("importOperationsFromModule", [](smtk::operation::Manager& manager, const std::string& moduleName)
      {
         return smtk::operation::ImportPythonOperation::importOperationsFromModule(moduleName, manager);
      }, py::arg("module"))
    .def("managers", &smtk::operation::Manager::managers)
    .def("setManagers", &smtk::operation::Manager::setManagers, py::arg("managers"))
    .def("launch", [](smtk::operation::Manager& manager, const std::shared_ptr<smtk::operation::Operation>& op)
      {
        manager.launchers()(op);
      }, py::arg("operation"))
    ;
  return instance;
}

#endif
