//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_TypeSet_h
#define pybind_smtk_mesh_TypeSet_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/TypeSet.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::TypeSet > pybind11_init_smtk_mesh_TypeSet(py::module &m)
{
  PySharedPtrClass< smtk::mesh::TypeSet > instance(m, "TypeSet");
  instance
    .def(py::init<::smtk::mesh::TypeSet const &>())
    .def(py::init<>())
    .def(py::init<::smtk::mesh::CellTypes, bool, bool>())
    .def("__ne__", (bool (smtk::mesh::TypeSet::*)(::smtk::mesh::TypeSet const &) const) &smtk::mesh::TypeSet::operator!=)
    .def("deepcopy", (smtk::mesh::TypeSet & (smtk::mesh::TypeSet::*)(::smtk::mesh::TypeSet const &)) &smtk::mesh::TypeSet::operator=)
    .def("__eq__", (bool (smtk::mesh::TypeSet::*)(::smtk::mesh::TypeSet const &) const) &smtk::mesh::TypeSet::operator==)
    .def("cellTypes", &smtk::mesh::TypeSet::cellTypes)
    .def("hasCell", &smtk::mesh::TypeSet::hasCell, py::arg("ct"))
    .def("hasCells", &smtk::mesh::TypeSet::hasCells)
    .def("hasDimension", &smtk::mesh::TypeSet::hasDimension, py::arg("dt"))
    .def("hasMeshes", &smtk::mesh::TypeSet::hasMeshes)
    ;
  return instance;
}

#endif
