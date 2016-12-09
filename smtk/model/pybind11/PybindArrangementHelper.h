//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_ArrangementHelper_h
#define pybind_smtk_model_ArrangementHelper_h

#include <pybind11/pybind11.h>

#include "smtk/model/ArrangementHelper.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Session.h"

namespace py = pybind11;

py::class_< smtk::model::ArrangementHelper > pybind11_init_smtk_model_ArrangementHelper(py::module &m)
{
  py::class_< smtk::model::ArrangementHelper > instance(m, "ArrangementHelper");
  instance
    .def("classname", &smtk::model::ArrangementHelper::classname)
    .def("doneAddingEntities", &smtk::model::ArrangementHelper::doneAddingEntities, py::arg("sess"), py::arg("flags"))
    .def("isMarked", &smtk::model::ArrangementHelper::isMarked, py::arg("ent"))
    .def("mark", &smtk::model::ArrangementHelper::mark, py::arg("ent"), py::arg("m"))
    .def("reset", &smtk::model::ArrangementHelper::reset, py::arg("ent"))
    .def("resetMarks", &smtk::model::ArrangementHelper::resetMarks)
    ;
  return instance;
}

#endif
