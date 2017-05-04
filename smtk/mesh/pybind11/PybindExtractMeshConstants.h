//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_ExtractMeshConstants_h
#define pybind_smtk_mesh_ExtractMeshConstants_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/ExtractMeshConstants.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::PreAllocatedMeshConstants > pybind11_init_smtk_mesh_PreAllocatedMeshConstants(py::module &m)
{
  PySharedPtrClass< smtk::mesh::PreAllocatedMeshConstants > instance(m, "PreAllocatedMeshConstants");
  instance
    .def(py::init<::smtk::mesh::PreAllocatedMeshConstants const &>())
    .def(py::init<::int64_t *, ::int64_t *>())
    .def("deepcopy", (smtk::mesh::PreAllocatedMeshConstants & (smtk::mesh::PreAllocatedMeshConstants::*)(::smtk::mesh::PreAllocatedMeshConstants const &)) &smtk::mesh::PreAllocatedMeshConstants::operator=)
    .def_static("determineAllocationLengths", &smtk::mesh::PreAllocatedMeshConstants::determineAllocationLengths, py::arg("ms"), py::arg("numberOfCells"), py::arg("numberOfPoints"))
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::MeshConstants > pybind11_init_smtk_mesh_MeshConstants(py::module &m)
{
  PySharedPtrClass< smtk::mesh::MeshConstants > instance(m, "MeshConstants");
  instance
    .def(py::init<::smtk::mesh::MeshConstants const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::mesh::MeshConstants & (smtk::mesh::MeshConstants::*)(::smtk::mesh::MeshConstants const &)) &smtk::mesh::MeshConstants::operator=)
    .def("cellData", &smtk::mesh::MeshConstants::cellData)
    .def("extractDirichlet", (void (smtk::mesh::MeshConstants::*)(::smtk::mesh::MeshSet const &)) &smtk::mesh::MeshConstants::extractDirichlet, py::arg("ms"))
    .def("extractDirichlet", (void (smtk::mesh::MeshConstants::*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &)) &smtk::mesh::MeshConstants::extractDirichlet, py::arg("cs"), py::arg("ps"))
    .def("extractDomain", (void (smtk::mesh::MeshConstants::*)(::smtk::mesh::MeshSet const &)) &smtk::mesh::MeshConstants::extractDomain, py::arg("ms"))
    .def("extractDomain", (void (smtk::mesh::MeshConstants::*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &)) &smtk::mesh::MeshConstants::extractDomain, py::arg("cs"), py::arg("ps"))
    .def("extractNeumann", (void (smtk::mesh::MeshConstants::*)(::smtk::mesh::MeshSet const &)) &smtk::mesh::MeshConstants::extractNeumann, py::arg("ms"))
    .def("extractNeumann", (void (smtk::mesh::MeshConstants::*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &)) &smtk::mesh::MeshConstants::extractNeumann, py::arg("cs"), py::arg("ps"))
    .def("pointData", &smtk::mesh::MeshConstants::pointData)
    ;
  return instance;
}

void pybind11_init__ZN4smtk4mesh21extractDirichletMeshConstantsERKNS0_7MeshSetERNS0_17PreAllocatedMeshConstantsE(py::module &m)
{
  m.def("extractDirichletMeshConstants", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PreAllocatedMeshConstants &)) &smtk::mesh::extractDirichletMeshConstants, "", py::arg("arg0"), py::arg("arg1"));
}

void pybind11_init__ZN4smtk4mesh21extractDirichletMeshConstantsERKNS0_7MeshSetERKNS0_8PointSetERNS0_17PreAllocatedMeshConstantsE(py::module &m)
{
  m.def("extractDirichletMeshConstants", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &, ::smtk::mesh::PreAllocatedMeshConstants &)) &smtk::mesh::extractDirichletMeshConstants, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

void pybind11_init__ZN4smtk4mesh18extractDomainMeshConstantsERKNS0_7MeshSetERNS0_17PreAllocatedMeshConstantsE(py::module &m)
{
  m.def("extractDomainMeshConstants", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PreAllocatedMeshConstants &)) &smtk::mesh::extractDomainMeshConstants, "", py::arg("arg0"), py::arg("arg1"));
}

void pybind11_init__ZN4smtk4mesh18extractDomainMeshConstantsERKNS0_7MeshSetERKNS0_8PointSetERNS0_17PreAllocatedMeshConstantsE(py::module &m)
{
  m.def("extractDomainMeshConstants", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &, ::smtk::mesh::PreAllocatedMeshConstants &)) &smtk::mesh::extractDomainMeshConstants, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

void pybind11_init__ZN4smtk4mesh19extractNeumannMeshConstantsERKNS0_7MeshSetERNS0_17PreAllocatedMeshConstantsE(py::module &m)
{
  m.def("extractNeumannMeshConstants", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PreAllocatedMeshConstants &)) &smtk::mesh::extractNeumannMeshConstants, "", py::arg("arg0"), py::arg("arg1"));
}

void pybind11_init__ZN4smtk4mesh19extractNeumannMeshConstantsERKNS0_7MeshSetERKNS0_8PointSetERNS0_17PreAllocatedMeshConstantsE(py::module &m)
{
  m.def("extractNeumannMeshConstants", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &, ::smtk::mesh::PreAllocatedMeshConstants &)) &smtk::mesh::extractNeumannMeshConstants, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

#endif
