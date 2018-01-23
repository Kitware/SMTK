//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_SplitEdge_h
#define pybind_smtk_bridge_polygon_operators_SplitEdge_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/SplitEdge.h"

#include "smtk/bridge/polygon/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::SplitEdge > pybind11_init_smtk_bridge_polygon_SplitEdge(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operator, smtk::operation::XMLOperator >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::SplitEdge > instance(m, "SplitEdge", parent);
  instance
    .def(py::init<::smtk::bridge::polygon::SplitEdge const &>())
    .def("deepcopy", (smtk::bridge::polygon::SplitEdge & (smtk::bridge::polygon::SplitEdge::*)(::smtk::bridge::polygon::SplitEdge const &)) &smtk::bridge::polygon::SplitEdge::operator=)
    .def("classname", &smtk::bridge::polygon::SplitEdge::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::SplitEdge> (*)()) &smtk::bridge::polygon::SplitEdge::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::SplitEdge> (*)(::std::shared_ptr<smtk::bridge::polygon::SplitEdge> &)) &smtk::bridge::polygon::SplitEdge::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::polygon::SplitEdge> (smtk::bridge::polygon::SplitEdge::*)() const) &smtk::bridge::polygon::SplitEdge::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::polygon::SplitEdge> (smtk::bridge::polygon::SplitEdge::*)()) &smtk::bridge::polygon::SplitEdge::shared_from_this)
    ;
  return instance;
}

#endif
