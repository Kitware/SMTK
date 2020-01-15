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

#include "smtk/project/old/Project.h"

#include "smtk/attribute/Resource.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/model/Resource.h"
#include "smtk/resource/Resource.h"

namespace py = pybind11;

PySharedPtrClass<smtk::project::old::Project> pybind11_init_smtk_project_Project(py::module& m)
{
  PySharedPtrClass<smtk::project::old::Project> instance(m, "Project");
  instance.def(py::init< ::smtk::project::old::Project const&>())
    .def("simulationCode", &smtk::project::old::Project::simulationCode)
    .def("name", &smtk::project::old::Project::name)
    .def("directory", &smtk::project::old::Project::directory)
    .def("resources", &smtk::project::old::Project::resources)
    .def("importLocation", &smtk::project::old::Project::importLocation)
    .def("addModel", &smtk::project::old::Project::addModel)

    .def("findAttributeResource", [](smtk::project::old::Project& prj, const std::string& identifier) {
        return prj.findResource<smtk::attribute::Resource>(identifier);
      })
    .def("findMeshResource", [](smtk::project::old::Project& prj, const std::string& identifier) {
        return prj.findResource<smtk::mesh::Resource>(identifier);
      })
    .def("findModelResource", [](smtk::project::old::Project& prj, const std::string& identifier) {
        return prj.findResource<smtk::model::Resource>(identifier);
      })
    ;
  return instance;
}

#endif
