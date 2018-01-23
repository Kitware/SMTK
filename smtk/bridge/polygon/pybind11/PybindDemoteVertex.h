//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_DemoteVertex_h
#define pybind_smtk_bridge_polygon_operators_DemoteVertex_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/DemoteVertex.h"

#include "smtk/bridge/polygon/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::DemoteVertex > pybind11_init_smtk_bridge_polygon_DemoteVertex(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::operation::XMLOperator >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::DemoteVertex > instance(m, "DemoteVertex", parent);
  instance
    .def(py::init<::smtk::bridge::polygon::DemoteVertex const &>())
    .def("deepcopy", (smtk::bridge::polygon::DemoteVertex & (smtk::bridge::polygon::DemoteVertex::*)(::smtk::bridge::polygon::DemoteVertex const &)) &smtk::bridge::polygon::DemoteVertex::operator=)
    .def("classname", &smtk::bridge::polygon::DemoteVertex::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::DemoteVertex> (*)()) &smtk::bridge::polygon::DemoteVertex::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::DemoteVertex> (*)(::std::shared_ptr<smtk::bridge::polygon::DemoteVertex> &)) &smtk::bridge::polygon::DemoteVertex::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::DemoteVertex> (smtk::bridge::polygon::DemoteVertex::*)() const) &smtk::bridge::polygon::DemoteVertex::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::DemoteVertex> (smtk::bridge::polygon::DemoteVertex::*)()) &smtk::bridge::polygon::DemoteVertex::shared_from_this)
    ;
  return instance;
}

#endif
