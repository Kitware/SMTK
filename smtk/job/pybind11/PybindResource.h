//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_job_Resource_h
#define pybind_smtk_job_Resource_h

#include "smtk/job/Manager.h"
#include "smtk/job/Resource.h"

namespace py = pybind11;

inline PySharedPtrClass<smtk::job::Resource, smtk::resource::Resource> pybind11_init_smtk_job_Resource(py::module &m)
{
  PySharedPtrClass<smtk::job::Resource, smtk::resource::Resource> instance(m, "Resource");
  instance
    .def_static("instance", &smtk::job::Resource::instance)
    .def("addJob", &smtk::job::Resource::addJob, py::arg("job"))
    .def("findJob", &smtk::job::Resource::findJob, py::arg("id"))
    ;
  return instance;
}

#endif
