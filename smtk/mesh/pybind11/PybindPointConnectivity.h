//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_PointConnectivity_h
#define pybind_smtk_mesh_PointConnectivity_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/PointConnectivity.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::PointConnectivity > pybind11_init_smtk_mesh_PointConnectivity(py::module &m)
{
  PySharedPtrClass< smtk::mesh::PointConnectivity > instance(m, "PointConnectivity");
  instance
    .def(py::init<::smtk::mesh::CollectionPtr const &, ::smtk::mesh::HandleRange const &>())
    .def(py::init<::smtk::mesh::PointConnectivity const &>())
    .def("__ne__", (bool (smtk::mesh::PointConnectivity::*)(::smtk::mesh::PointConnectivity const &) const) &smtk::mesh::PointConnectivity::operator!=)
    .def("deepcopy", (smtk::mesh::PointConnectivity & (smtk::mesh::PointConnectivity::*)(::smtk::mesh::PointConnectivity const &)) &smtk::mesh::PointConnectivity::operator=)
    .def("__eq__", (bool (smtk::mesh::PointConnectivity::*)(::smtk::mesh::PointConnectivity const &) const) &smtk::mesh::PointConnectivity::operator==)
    // .def("fetchNextCell", (bool (smtk::mesh::PointConnectivity::*)(int &, ::smtk::mesh::Handle const * &)) &smtk::mesh::PointConnectivity::fetchNextCell, py::arg("numPts"), py::arg("points"))
    // .def("fetchNextCell", (bool (smtk::mesh::PointConnectivity::*)(::smtk::mesh::CellType &, int &, ::smtk::mesh::Handle const * &)) &smtk::mesh::PointConnectivity::fetchNextCell, py::arg("cellType"), py::arg("numPts"), py::arg("points"))
    .def("initCellTraversal", &smtk::mesh::PointConnectivity::initCellTraversal)
    .def("is_empty", &smtk::mesh::PointConnectivity::is_empty)
    .def("numberOfCells", &smtk::mesh::PointConnectivity::numberOfCells)
    .def("size", &smtk::mesh::PointConnectivity::size)
    ;
  return instance;
}

#endif
