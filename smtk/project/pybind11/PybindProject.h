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

#include "smtk/attribute/Resource.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/model/Resource.h"
#include "smtk/resource/Resource.h"

namespace py = pybind11;

PySharedPtrClass<smtk::project::Project> pybind11_init_smtk_project_Project(py::module& m)
{
  PySharedPtrClass<smtk::project::Project> instance(m, "Project");
  instance.def(py::init< ::smtk::project::Project const&>())
    .def("simulationCode", &smtk::project::Project::simulationCode)
    .def("name", &smtk::project::Project::name)
    .def("directory", &smtk::project::Project::directory)
    .def("resources", &smtk::project::Project::resources)
    .def("importLocation", &smtk::project::Project::importLocation)

    .def("findAttributeResource", [](smtk::project::Project& prj, const std::string& identifier) {
        return prj.findResource<smtk::attribute::Resource>(identifier);
      })
    .def("findMeshResource", [](smtk::project::Project& prj, const std::string& identifier) {
        return prj.findResource<smtk::mesh::Resource>(identifier);
      })
    .def("findModelResource", [](smtk::project::Project& prj, const std::string& identifier) {
        return prj.findResource<smtk::model::Resource>(identifier);
      })
    ;
  return instance;
}

#endif
