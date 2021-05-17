//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_project_OperationFactory_h
#define pybind_smtk_project_OperationFactory_h

#include <pybind11/pybind11.h>

#include "smtk/project/OperationFactory.h"

#include "smtk/resource/Component.h"

namespace py = pybind11;

inline py::class_< smtk::project::OperationFactory > pybind11_init_smtk_project_OperationFactory(py::module &m)
{
  py::class_< smtk::project::OperationFactory > instance(m, "OperationFactory");
  instance
    .def(py::init<::std::weak_ptr<smtk::operation::Manager> const &>())
    .def(py::init<::smtk::project::OperationFactory const &>())
    .def("deepcopy", (smtk::project::OperationFactory & (smtk::project::OperationFactory::*)(::smtk::project::OperationFactory const &)) &smtk::project::OperationFactory::operator=)
    .def("registerOperation", (bool (smtk::project::OperationFactory::*)(::std::string const &)) &smtk::project::OperationFactory::registerOperation, py::arg("arg0"))
    .def("registerOperation", (bool (smtk::project::OperationFactory::*)(::smtk::operation::Operation::Index const &)) &smtk::project::OperationFactory::registerOperation, py::arg("arg0"))
    .def("registerOperations", &smtk::project::OperationFactory::registerOperations, py::arg("arg0"))
    .def("unregisterOperation", (bool (smtk::project::OperationFactory::*)(::std::string const &)) &smtk::project::OperationFactory::unregisterOperation, py::arg("arg0"))
    .def("unregisterOperation", (bool (smtk::project::OperationFactory::*)(::smtk::operation::Operation::Index const &)) &smtk::project::OperationFactory::unregisterOperation, py::arg("arg0"))
    .def("create", (std::shared_ptr<smtk::operation::Operation> (smtk::project::OperationFactory::*)(::std::string const &)) &smtk::project::OperationFactory::create, py::arg("arg0"))
    .def("create", (std::shared_ptr<smtk::operation::Operation> (smtk::project::OperationFactory::*)(::smtk::operation::Operation::Index const &)) &smtk::project::OperationFactory::create, py::arg("arg0"))
    .def("types", (std::set<std::basic_string<char>, std::less<std::basic_string<char> >, std::allocator<std::basic_string<char> > > const & (smtk::project::OperationFactory::*)() const) &smtk::project::OperationFactory::types)
    .def("types", (std::set<std::basic_string<char>, std::less<std::basic_string<char> >, std::allocator<std::basic_string<char> > > & (smtk::project::OperationFactory::*)()) &smtk::project::OperationFactory::types)
    .def("availableOperations", &smtk::project::OperationFactory::availableOperations, py::arg("arg0"))
    .def("manager", &smtk::project::OperationFactory::manager)
    .def("setManager", &smtk::project::OperationFactory::setManager, py::arg("manager"))
    ;
  return instance;
}

#endif
