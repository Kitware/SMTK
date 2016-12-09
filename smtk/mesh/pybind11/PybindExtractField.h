//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_ExtractField_h
#define pybind_smtk_mesh_ExtractField_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/ExtractField.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::PreAllocatedField > pybind11_init_smtk_mesh_PreAllocatedField(py::module &m)
{
  PySharedPtrClass< smtk::mesh::PreAllocatedField > instance(m, "PreAllocatedField");
  instance
    .def(py::init<::smtk::mesh::PreAllocatedField const &>())
    .def(py::init<::int64_t *, ::int64_t *>())
    .def("deepcopy", (smtk::mesh::PreAllocatedField & (smtk::mesh::PreAllocatedField::*)(::smtk::mesh::PreAllocatedField const &)) &smtk::mesh::PreAllocatedField::operator=)
    .def_static("determineAllocationLengths", &smtk::mesh::PreAllocatedField::determineAllocationLengths, py::arg("ms"), py::arg("numberOfCells"), py::arg("numberOfPoints"))
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::Field > pybind11_init_smtk_mesh_Field(py::module &m)
{
  PySharedPtrClass< smtk::mesh::Field > instance(m, "Field");
  instance
    .def(py::init<::smtk::mesh::Field const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::mesh::Field & (smtk::mesh::Field::*)(::smtk::mesh::Field const &)) &smtk::mesh::Field::operator=)
    .def("cellData", &smtk::mesh::Field::cellData)
    .def("extractDirichlet", (void (smtk::mesh::Field::*)(::smtk::mesh::MeshSet const &)) &smtk::mesh::Field::extractDirichlet, py::arg("ms"))
    .def("extractDirichlet", (void (smtk::mesh::Field::*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &)) &smtk::mesh::Field::extractDirichlet, py::arg("cs"), py::arg("ps"))
    .def("extractDomain", (void (smtk::mesh::Field::*)(::smtk::mesh::MeshSet const &)) &smtk::mesh::Field::extractDomain, py::arg("ms"))
    .def("extractDomain", (void (smtk::mesh::Field::*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &)) &smtk::mesh::Field::extractDomain, py::arg("cs"), py::arg("ps"))
    .def("extractNeumann", (void (smtk::mesh::Field::*)(::smtk::mesh::MeshSet const &)) &smtk::mesh::Field::extractNeumann, py::arg("ms"))
    .def("extractNeumann", (void (smtk::mesh::Field::*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &)) &smtk::mesh::Field::extractNeumann, py::arg("cs"), py::arg("ps"))
    .def("pointData", &smtk::mesh::Field::pointData)
    ;
  return instance;
}

void pybind11_init__ZN4smtk4mesh21extractDirichletFieldERKNS0_7MeshSetERNS0_17PreAllocatedFieldE(py::module &m)
{
  m.def("extractDirichletField", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PreAllocatedField &)) &smtk::mesh::extractDirichletField, "", py::arg("arg0"), py::arg("arg1"));
}

void pybind11_init__ZN4smtk4mesh21extractDirichletFieldERKNS0_7MeshSetERKNS0_8PointSetERNS0_17PreAllocatedFieldE(py::module &m)
{
  m.def("extractDirichletField", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &, ::smtk::mesh::PreAllocatedField &)) &smtk::mesh::extractDirichletField, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

void pybind11_init__ZN4smtk4mesh18extractDomainFieldERKNS0_7MeshSetERNS0_17PreAllocatedFieldE(py::module &m)
{
  m.def("extractDomainField", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PreAllocatedField &)) &smtk::mesh::extractDomainField, "", py::arg("arg0"), py::arg("arg1"));
}

void pybind11_init__ZN4smtk4mesh18extractDomainFieldERKNS0_7MeshSetERKNS0_8PointSetERNS0_17PreAllocatedFieldE(py::module &m)
{
  m.def("extractDomainField", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &, ::smtk::mesh::PreAllocatedField &)) &smtk::mesh::extractDomainField, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

void pybind11_init__ZN4smtk4mesh19extractNeumannFieldERKNS0_7MeshSetERNS0_17PreAllocatedFieldE(py::module &m)
{
  m.def("extractNeumannField", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PreAllocatedField &)) &smtk::mesh::extractNeumannField, "", py::arg("arg0"), py::arg("arg1"));
}

void pybind11_init__ZN4smtk4mesh19extractNeumannFieldERKNS0_7MeshSetERKNS0_8PointSetERNS0_17PreAllocatedFieldE(py::module &m)
{
  m.def("extractNeumannField", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &, ::smtk::mesh::PreAllocatedField &)) &smtk::mesh::extractNeumannField, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

#endif
