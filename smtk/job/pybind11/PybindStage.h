//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_job_Stage_h
#define pybind_smtk_job_Stage_h

#include "smtk/job/Stage.h"
#include "smtk/job/Queue.h"
#include "smtk/job/State.h"
#include "smtk/job/Status.h"

namespace py = pybind11;

inline PySharedPtrClass<smtk::job::Stage> pybind11_init_smtk_job_Stage(py::module &m)
{
  PySharedPtrClass<smtk::job::Stage> instance(m, "Stage");
  instance
    .def_static("create", (std::shared_ptr<smtk::job::Stage> (*)()) &smtk::job::Stage::create)
    .def("index", &smtk::job::Stage::index)

    .def("name", &smtk::job::Stage::name)
    .def("description", &smtk::job::Stage::description)

    .def("log", &smtk::job::Stage::log)
    .def("setLog", &smtk::job::Stage::setLog, py::arg("log_file"))
    ;
  return instance;
}

#endif
