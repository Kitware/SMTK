//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_QueryTypes_h
#define pybind_smtk_mesh_QueryTypes_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/QueryTypes.h"

namespace py = pybind11;

void pybind11_init_smtk_mesh_ContainmentType(py::module &m)
{
  py::enum_<smtk::mesh::ContainmentType>(m, "ContainmentType")
    .value("PartiallyContained", smtk::mesh::ContainmentType::PartiallyContained)
    .value("FullyContained", smtk::mesh::ContainmentType::FullyContained)
    .export_values();
}

PySharedPtrClass< smtk::mesh::Dirichlet > pybind11_init_smtk_mesh_Dirichlet(py::module &m, PySharedPtrClass< smtk::mesh::IntegerTag >& parent)
{
  PySharedPtrClass< smtk::mesh::Dirichlet > instance(m, "Dirichlet", parent);
  instance
    .def(py::init<::smtk::mesh::Dirichlet const &>())
    .def(py::init<int>())
    .def("deepcopy", (smtk::mesh::Dirichlet & (smtk::mesh::Dirichlet::*)(::smtk::mesh::Dirichlet const &)) &smtk::mesh::Dirichlet::operator=)
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::Domain > pybind11_init_smtk_mesh_Domain(py::module &m, PySharedPtrClass< smtk::mesh::IntegerTag >& parent)
{
  PySharedPtrClass< smtk::mesh::Domain > instance(m, "Domain", parent);
  instance
    .def(py::init<::smtk::mesh::Domain const &>())
    .def(py::init<int>())
    .def("deepcopy", (smtk::mesh::Domain & (smtk::mesh::Domain::*)(::smtk::mesh::Domain const &)) &smtk::mesh::Domain::operator=)
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::IntegerTag > pybind11_init_smtk_mesh_IntegerTag(py::module &m)
{
  PySharedPtrClass< smtk::mesh::IntegerTag > instance(m, "IntegerTag");
  instance
    .def(py::init<::smtk::mesh::IntegerTag const &>())
    .def(py::init<int>())
    .def("__ne__", (bool (smtk::mesh::IntegerTag::*)(::smtk::mesh::IntegerTag const &) const) &smtk::mesh::IntegerTag::operator!=)
    .def("__lt__", (bool (smtk::mesh::IntegerTag::*)(::smtk::mesh::IntegerTag const &) const) &smtk::mesh::IntegerTag::operator<)
    .def("deepcopy", (smtk::mesh::IntegerTag & (smtk::mesh::IntegerTag::*)(::smtk::mesh::IntegerTag const &)) &smtk::mesh::IntegerTag::operator=)
    .def("__eq__", (bool (smtk::mesh::IntegerTag::*)(::smtk::mesh::IntegerTag const &) const) &smtk::mesh::IntegerTag::operator==)
    .def("value", &smtk::mesh::IntegerTag::value)
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::Model > pybind11_init_smtk_mesh_Model(py::module &m, PySharedPtrClass< smtk::mesh::UUIDTag >& parent)
{
  PySharedPtrClass< smtk::mesh::Model > instance(m, "Model", parent);
  instance
    .def(py::init<::smtk::mesh::Model const &>())
    .def(py::init<::smtk::common::UUID const &>())
    .def("deepcopy", (smtk::mesh::Model & (smtk::mesh::Model::*)(::smtk::mesh::Model const &)) &smtk::mesh::Model::operator=)
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::Neumann > pybind11_init_smtk_mesh_Neumann(py::module &m, PySharedPtrClass< smtk::mesh::IntegerTag >& parent)
{
  PySharedPtrClass< smtk::mesh::Neumann > instance(m, "Neumann", parent);
  instance
    .def(py::init<::smtk::mesh::Neumann const &>())
    .def(py::init<int>())
    .def("deepcopy", (smtk::mesh::Neumann & (smtk::mesh::Neumann::*)(::smtk::mesh::Neumann const &)) &smtk::mesh::Neumann::operator=)
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::OpaqueTag<16> > pybind11_init_smtk_mesh_OpaqueTag_16_(py::module &m)
{
  PySharedPtrClass< smtk::mesh::OpaqueTag<16> > instance(m, "OpaqueTag_16_");
  instance
    .def(py::init<::smtk::mesh::OpaqueTag<16> const &>())
    .def(py::init<unsigned char const *>())
    .def("__ne__", (bool (smtk::mesh::OpaqueTag<16>::*)(::smtk::mesh::OpaqueTag<16> const &) const) &smtk::mesh::OpaqueTag<16>::operator!=)
    .def("__lt__", (bool (smtk::mesh::OpaqueTag<16>::*)(::smtk::mesh::OpaqueTag<16> const &) const) &smtk::mesh::OpaqueTag<16>::operator<)
    .def("deepcopy", (smtk::mesh::OpaqueTag<16> & (smtk::mesh::OpaqueTag<16>::*)(::smtk::mesh::OpaqueTag<16> const &)) &smtk::mesh::OpaqueTag<16>::operator=)
    .def("__eq__", (bool (smtk::mesh::OpaqueTag<16>::*)(::smtk::mesh::OpaqueTag<16> const &) const) &smtk::mesh::OpaqueTag<16>::operator==)
    .def_static("size", &smtk::mesh::OpaqueTag<16>::size)
    .def("value", &smtk::mesh::OpaqueTag<16>::value)
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::UUIDTag > pybind11_init_smtk_mesh_UUIDTag(py::module &m, PySharedPtrClass< smtk::mesh::OpaqueTag<16> >& parent)
{
  PySharedPtrClass< smtk::mesh::UUIDTag > instance(m, "UUIDTag", parent);
  instance
    .def(py::init<::smtk::mesh::UUIDTag const &>())
    .def(py::init<::smtk::common::UUID const &>())
    .def("deepcopy", (smtk::mesh::UUIDTag & (smtk::mesh::UUIDTag::*)(::smtk::mesh::UUIDTag const &)) &smtk::mesh::UUIDTag::operator=)
    .def("uuid", &smtk::mesh::UUIDTag::uuid)
    ;
  return instance;
}

#endif
