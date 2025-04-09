//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_CreateFaces_h
#define pybind_smtk_session_polygon_operators_CreateFaces_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/operators/CreateFaces.h"

namespace py = pybind11;

inline py::class_< smtk::session::polygon::ModelEdgeInfo > pybind11_init_smtk_session_polygon_ModelEdgeInfo(py::module &m)
{
  py::class_< smtk::session::polygon::ModelEdgeInfo > instance(m, "ModelEdgeInfo");
  instance
    .def(py::init<>())
    .def(py::init<int>())
    .def(py::init<::smtk::session::polygon::ModelEdgeInfo const &>())
    .def("deepcopy", (smtk::session::polygon::ModelEdgeInfo & (smtk::session::polygon::ModelEdgeInfo::*)(::smtk::session::polygon::ModelEdgeInfo const &)) &smtk::session::polygon::ModelEdgeInfo::operator=)
    .def_readwrite("m_allowedOrientations", &smtk::session::polygon::ModelEdgeInfo::m_allowedOrientations)
    ;
  return instance;
}

inline PySharedPtrClass< smtk::session::polygon::CreateFaces > pybind11_init_smtk_session_polygon_CreateFaces(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::CreateFaces > instance(m, "CreateFaces", parent);
  instance
    .def_static("create", (std::shared_ptr<smtk::session::polygon::CreateFaces> (*)()) &smtk::session::polygon::CreateFaces::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::CreateFaces> (*)(::std::shared_ptr<smtk::session::polygon::CreateFaces> &)) &smtk::session::polygon::CreateFaces::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::CreateFaces> (smtk::session::polygon::CreateFaces::*)() const) &smtk::session::polygon::CreateFaces::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::CreateFaces> (smtk::session::polygon::CreateFaces::*)()) &smtk::session::polygon::CreateFaces::shared_from_this)
    ;
  return instance;
}

#endif
