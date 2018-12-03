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
#include "smtk/operation/Manager.h"
#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "PybindProject.h"

#include <vector>

namespace py = pybind11;

PySharedPtrClass<smtk::project::Manager> pybind11_init_smtk_project_Manager(py::module& m)
{
  PySharedPtrClass<smtk::project::Manager> instance(m, "Manager");
  instance.def(py::init< ::smtk::project::Manager const&>())
    .def_static(
      "create", (std::shared_ptr<smtk::project::Manager>(*)()) & smtk::project::Manager::create)
    .def_static("create",
      (std::shared_ptr<smtk::project::Manager>(*)(std::shared_ptr<smtk::project::Manager>&)) &
        smtk::project::Manager::create)
    .def("setCoreManagers", &smtk::project::Manager::setCoreManagers)
    .def("getProjectSpecification", &smtk::project::Manager::getProjectSpecification)
    .def("createProject", &smtk::project::Manager::createProject)
    .def("getCurrentProject", &smtk::project::Manager::getCurrentProject)
    .def("saveProject", &smtk::project::Manager::saveProject)
    .def("closeProject", &smtk::project::Manager::closeProject)
    .def("openProject", &smtk::project::Manager::openProject);
  return instance;
}

#endif
