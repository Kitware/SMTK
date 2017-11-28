//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_ForceCreateFace_h
#define pybind_smtk_bridge_polygon_operators_ForceCreateFace_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/ForceCreateFace.h"

#include "smtk/bridge/polygon/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::ForceCreateFace > pybind11_init_smtk_bridge_polygon_ForceCreateFace(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::operation::XMLOperator >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::ForceCreateFace > instance(m, "ForceCreateFace", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::polygon::ForceCreateFace const &>())
    .def("deepcopy", (smtk::bridge::polygon::ForceCreateFace & (smtk::bridge::polygon::ForceCreateFace::*)(::smtk::bridge::polygon::ForceCreateFace const &)) &smtk::bridge::polygon::ForceCreateFace::operator=)
    .def("ableToOperate", &smtk::bridge::polygon::ForceCreateFace::ableToOperate)
    .def("classname", &smtk::bridge::polygon::ForceCreateFace::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::ForceCreateFace> (*)()) &smtk::bridge::polygon::ForceCreateFace::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::ForceCreateFace> (*)(::std::shared_ptr<smtk::bridge::polygon::ForceCreateFace> &)) &smtk::bridge::polygon::ForceCreateFace::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::ForceCreateFace> (smtk::bridge::polygon::ForceCreateFace::*)() const) &smtk::bridge::polygon::ForceCreateFace::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::ForceCreateFace> (smtk::bridge::polygon::ForceCreateFace::*)()) &smtk::bridge::polygon::ForceCreateFace::shared_from_this)
    ;
  py::enum_<smtk::bridge::polygon::ForceCreateFace::ConstructionMethod>(instance, "ConstructionMethod")
    .value("POINTS", smtk::bridge::polygon::ForceCreateFace::ConstructionMethod::POINTS)
    .value("EDGES", smtk::bridge::polygon::ForceCreateFace::ConstructionMethod::EDGES)
    .export_values();
  return instance;
}

#endif
