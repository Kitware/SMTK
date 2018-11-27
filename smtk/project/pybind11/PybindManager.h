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
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

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
    .def("setManagers", &smtk::project::Manager::setManagers)
    .def("getProjectSpecification", &smtk::project::Manager::getProjectSpecification)
    .def("createProject", &smtk::project::Manager::createProject)
    .def("getStatus", &smtk::project::Manager::getStatus)
    .def("getResourceByRole", &smtk::project::Manager::getResourceByRole)
    .def("getResourceInfos", &smtk::project::Manager::getResourceInfos)
    .def("saveProject", &smtk::project::Manager::saveProject)
    .def("closeProject", &smtk::project::Manager::closeProject)
    .def("openProject", &smtk::project::Manager::openProject)
    .def("getExportTemplate", &smtk::project::Manager::getExportTemplate)
    .def("exportProject", &smtk::project::Manager::exportProject);
  return instance;
}

#endif
