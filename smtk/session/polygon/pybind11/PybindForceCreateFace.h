//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_ForceCreateFace_h
#define pybind_smtk_session_polygon_operators_ForceCreateFace_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/operators/ForceCreateFace.h"

#include "smtk/session/polygon/Operation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::ForceCreateFace > pybind11_init_smtk_session_polygon_ForceCreateFace(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::ForceCreateFace > instance(m, "ForceCreateFace", parent);
  instance
    .def(py::init<>())
    .def("ableToOperate", &smtk::session::polygon::ForceCreateFace::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::ForceCreateFace> (*)()) &smtk::session::polygon::ForceCreateFace::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::ForceCreateFace> (*)(::std::shared_ptr<smtk::session::polygon::ForceCreateFace> &)) &smtk::session::polygon::ForceCreateFace::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::ForceCreateFace> (smtk::session::polygon::ForceCreateFace::*)() const) &smtk::session::polygon::ForceCreateFace::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::ForceCreateFace> (smtk::session::polygon::ForceCreateFace::*)()) &smtk::session::polygon::ForceCreateFace::shared_from_this)
    ;
  py::enum_<smtk::session::polygon::ForceCreateFace::ConstructionMethod>(instance, "ConstructionMethod")
    .value("POINTS", smtk::session::polygon::ForceCreateFace::ConstructionMethod::POINTS)
    .value("EDGES", smtk::session::polygon::ForceCreateFace::ConstructionMethod::EDGES)
    .export_values();
  return instance;
}

#endif
