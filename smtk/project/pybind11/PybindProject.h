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
#include <pybind11/stl.h>

#include "smtk/project/Project.h"

namespace py = pybind11;

PySharedPtrClass<smtk::project::Project> pybind11_init_smtk_project_Project(py::module& m)
{
  PySharedPtrClass<smtk::project::Project> instance(m, "Project");
  instance.def(py::init< ::smtk::project::Project const&>())
    .def("name", &smtk::project::Project::name)
    .def("directory", &smtk::project::Project::directory)
    .def("getResourceInfos", &smtk::project::Project::getResourceInfos)
    .def("getResourceByRole", &smtk::project::Project::getResourceByRole);
  return instance;
}

#endif
