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
#include <pybind11/stl.h>

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/project/old/Manager.h"
#include "smtk/project/old/Project.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "PybindProject.h"

#include <vector>

namespace py = pybind11;

PySharedPtrClass<smtk::project::old::Manager> pybind11_init_smtk_project_Manager(py::module& m)
{
  PySharedPtrClass<smtk::project::old::Manager> instance(m, "Manager");
  instance.def(py::init< ::smtk::project::old::Manager const&>())
    .def_static("create", &smtk::project::old::Manager::create, py::arg("resourceManager"),
      py::arg("operationManager"))
    .def("getProjectSpecification", &smtk::project::old::Manager::getProjectSpecification)
    .def("createProject", &smtk::project::old::Manager::createProject, py::arg("specification"),
      py::arg("replaceExistingDirectory") = false, py::arg("logger") = smtk::io::Logger::instance())
    .def("getCurrentProject", &smtk::project::old::Manager::getCurrentProject)
    .def("saveProject", &smtk::project::old::Manager::saveProject,
      py::arg("logger") = smtk::io::Logger::instance())
    .def("closeProject", &smtk::project::old::Manager::closeProject,
      py::arg("logger") = smtk::io::Logger::instance())
    .def("openProject", &smtk::project::old::Manager::openProject, py::arg("path"),
      py::arg("logger") = smtk::io::Logger::instance());
  return instance;
}

#endif
