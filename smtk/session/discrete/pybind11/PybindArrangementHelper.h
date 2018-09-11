//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_discrete_ArrangementHelper_h
#define pybind_smtk_session_discrete_ArrangementHelper_h

#include <pybind11/pybind11.h>

#include "smtk/session/discrete/ArrangementHelper.h"

#include "smtk/model/ArrangementHelper.h"

#include "vtkModelEdgeUse.h"
#include "vtkModelRegion.h"

namespace py = pybind11;

py::class_< smtk::session::discrete::ArrangementHelper, smtk::model::ArrangementHelper > pybind11_init_smtk_session_discrete_ArrangementHelper(py::module &m)
{
  py::class_< smtk::session::discrete::ArrangementHelper, smtk::model::ArrangementHelper > instance(m, "ArrangementHelper");
  instance
    .def(py::init<>())
    .def("addArrangement", (void (smtk::session::discrete::ArrangementHelper::*)(::smtk::model::EntityRef const &, ::smtk::model::ArrangementKind, ::smtk::model::EntityRef const &)) &smtk::session::discrete::ArrangementHelper::addArrangement, py::arg("parent"), py::arg("k"), py::arg("child"))
    .def("addArrangement", (void (smtk::session::discrete::ArrangementHelper::*)(::smtk::model::EntityRef const &, ::smtk::model::ArrangementKind, ::smtk::model::EntityRef const &, int, ::smtk::model::Orientation, int)) &smtk::session::discrete::ArrangementHelper::addArrangement, py::arg("parent"), py::arg("k"), py::arg("child"), py::arg("sense"), py::arg("orientation"), py::arg("iter_pos") = 0)
    .def("resetArrangements", &smtk::session::discrete::ArrangementHelper::resetArrangements)
    .def("doneAddingEntities", &smtk::session::discrete::ArrangementHelper::doneAddingEntities, py::arg("baseSession"), py::arg("flags"))
    .def("findOrAssignSense", &smtk::session::discrete::ArrangementHelper::findOrAssignSense, py::arg("eu1"))
    .def("useForRegion", &smtk::session::discrete::ArrangementHelper::useForRegion, py::arg("arg0"))
    .def("regionFromUseId", &smtk::session::discrete::ArrangementHelper::regionFromUseId, py::arg("arg0"))
    .def("chainForEdgeUse", &smtk::session::discrete::ArrangementHelper::chainForEdgeUse, py::arg("arg0"))
    .def("edgeUseFromChainId", &smtk::session::discrete::ArrangementHelper::edgeUseFromChainId, py::arg("arg0"))
    ;
  return instance;
}

#endif
