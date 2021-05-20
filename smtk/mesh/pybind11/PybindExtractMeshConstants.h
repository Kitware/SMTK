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

#include "smtk/mesh/utility/ExtractMeshConstants.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::mesh::utility::PreAllocatedMeshConstants > pybind11_init_smtk_mesh_PreAllocatedMeshConstants(py::module &m)
{
  PySharedPtrClass< smtk::mesh::utility::PreAllocatedMeshConstants > instance(m, "PreAllocatedMeshConstants");
  instance
    .def(py::init<::smtk::mesh::utility::PreAllocatedMeshConstants const &>())
    .def(py::init<::int64_t *, ::int64_t *>())
    .def("deepcopy", (smtk::mesh::utility::PreAllocatedMeshConstants & (smtk::mesh::utility::PreAllocatedMeshConstants::*)(::smtk::mesh::utility::PreAllocatedMeshConstants const &)) &smtk::mesh::utility::PreAllocatedMeshConstants::operator=)
    .def_static("determineAllocationLengths", &smtk::mesh::utility::PreAllocatedMeshConstants::determineAllocationLengths, py::arg("ms"), py::arg("numberOfCells"), py::arg("numberOfPoints"))
    ;
  return instance;
}

inline PySharedPtrClass< smtk::mesh::utility::MeshConstants > pybind11_init_smtk_mesh_MeshConstants(py::module &m)
{
  PySharedPtrClass< smtk::mesh::utility::MeshConstants > instance(m, "MeshConstants");
  instance
    .def(py::init<::smtk::mesh::utility::MeshConstants const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::mesh::utility::MeshConstants & (smtk::mesh::utility::MeshConstants::*)(::smtk::mesh::utility::MeshConstants const &)) &smtk::mesh::utility::MeshConstants::operator=)
    .def("cellData", &smtk::mesh::utility::MeshConstants::cellData)
    .def("extractDirichlet", (void (smtk::mesh::utility::MeshConstants::*)(::smtk::mesh::MeshSet const &)) &smtk::mesh::utility::MeshConstants::extractDirichlet, py::arg("ms"))
    .def("extractDirichlet", (void (smtk::mesh::utility::MeshConstants::*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &)) &smtk::mesh::utility::MeshConstants::extractDirichlet, py::arg("cs"), py::arg("ps"))
    .def("extractDomain", (void (smtk::mesh::utility::MeshConstants::*)(::smtk::mesh::MeshSet const &)) &smtk::mesh::utility::MeshConstants::extractDomain, py::arg("ms"))
    .def("extractDomain", (void (smtk::mesh::utility::MeshConstants::*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &)) &smtk::mesh::utility::MeshConstants::extractDomain, py::arg("cs"), py::arg("ps"))
    .def("extractNeumann", (void (smtk::mesh::utility::MeshConstants::*)(::smtk::mesh::MeshSet const &)) &smtk::mesh::utility::MeshConstants::extractNeumann, py::arg("ms"))
    .def("extractNeumann", (void (smtk::mesh::utility::MeshConstants::*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &)) &smtk::mesh::utility::MeshConstants::extractNeumann, py::arg("cs"), py::arg("ps"))
    .def("pointData", &smtk::mesh::utility::MeshConstants::pointData)
    ;
  return instance;
}

inline void pybind11_init__ZN4smtk4mesh21extractDirichletMeshConstantsERKNS0_7MeshSetERNS0_17PreAllocatedMeshConstantsE(py::module &m)
{
  m.def("extractDirichletMeshConstants", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::utility::PreAllocatedMeshConstants &)) &smtk::mesh::utility::extractDirichletMeshConstants, "", py::arg("arg0"), py::arg("arg1"));
}

inline void pybind11_init__ZN4smtk4mesh21extractDirichletMeshConstantsERKNS0_7MeshSetERKNS0_8PointSetERNS0_17PreAllocatedMeshConstantsE(py::module &m)
{
  m.def("extractDirichletMeshConstants", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &, ::smtk::mesh::utility::PreAllocatedMeshConstants &)) &smtk::mesh::utility::extractDirichletMeshConstants, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

inline void pybind11_init__ZN4smtk4mesh18extractDomainMeshConstantsERKNS0_7MeshSetERNS0_17PreAllocatedMeshConstantsE(py::module &m)
{
  m.def("extractDomainMeshConstants", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::utility::PreAllocatedMeshConstants &)) &smtk::mesh::utility::extractDomainMeshConstants, "", py::arg("arg0"), py::arg("arg1"));
}

inline void pybind11_init__ZN4smtk4mesh18extractDomainMeshConstantsERKNS0_7MeshSetERKNS0_8PointSetERNS0_17PreAllocatedMeshConstantsE(py::module &m)
{
  m.def("extractDomainMeshConstants", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &, ::smtk::mesh::utility::PreAllocatedMeshConstants &)) &smtk::mesh::utility::extractDomainMeshConstants, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

inline void pybind11_init__ZN4smtk4mesh19extractNeumannMeshConstantsERKNS0_7MeshSetERNS0_17PreAllocatedMeshConstantsE(py::module &m)
{
  m.def("extractNeumannMeshConstants", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::utility::PreAllocatedMeshConstants &)) &smtk::mesh::utility::extractNeumannMeshConstants, "", py::arg("arg0"), py::arg("arg1"));
}

inline void pybind11_init__ZN4smtk4mesh19extractNeumannMeshConstantsERKNS0_7MeshSetERKNS0_8PointSetERNS0_17PreAllocatedMeshConstantsE(py::module &m)
{
  m.def("extractNeumannMeshConstants", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &, ::smtk::mesh::utility::PreAllocatedMeshConstants &)) &smtk::mesh::utility::extractNeumannMeshConstants, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

#endif
