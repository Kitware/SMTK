//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_ExtractTessellation_h
#define pybind_smtk_mesh_ExtractTessellation_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/ExtractTessellation.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Loop.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::PreAllocatedTessellation > pybind11_init_smtk_mesh_PreAllocatedTessellation(py::module &m)
{
  PySharedPtrClass< smtk::mesh::PreAllocatedTessellation > instance(m, "PreAllocatedTessellation");
  instance
    .def(py::init<::smtk::mesh::PreAllocatedTessellation const &>())
    .def(py::init<::int64_t *>())
    .def(py::init<::int64_t *, float *>())
    .def(py::init<::int64_t *, double *>())
    .def(py::init<::int64_t *, ::int64_t *, unsigned char *>())
    .def(py::init<::int64_t *, ::int64_t *, unsigned char *, float *>())
    .def(py::init<::int64_t *, ::int64_t *, unsigned char *, double *>())
    .def("deepcopy", (smtk::mesh::PreAllocatedTessellation & (smtk::mesh::PreAllocatedTessellation::*)(::smtk::mesh::PreAllocatedTessellation const &)) &smtk::mesh::PreAllocatedTessellation::operator=)
    .def_static("determineAllocationLengths", (void (*)(::smtk::mesh::MeshSet const &, ::int64_t &, ::int64_t &, ::int64_t &)) &smtk::mesh::PreAllocatedTessellation::determineAllocationLengths, py::arg("ms"), py::arg("connectivityLength"), py::arg("numberOfCells"), py::arg("numberOfPoints"))
    .def_static("determineAllocationLengths", (void (*)(::smtk::mesh::CellSet const &, ::int64_t &, ::int64_t &, ::int64_t &)) &smtk::mesh::PreAllocatedTessellation::determineAllocationLengths, py::arg("cs"), py::arg("connectivityLength"), py::arg("numberOfCells"), py::arg("numberOfPoints"))
    .def_static("determineAllocationLengths", (void (*)(::smtk::model::EntityRef const &, ::smtk::mesh::CollectionPtr const &, ::int64_t &, ::int64_t &, ::int64_t &)) &smtk::mesh::PreAllocatedTessellation::determineAllocationLengths, py::arg("eRef"), py::arg("c"), py::arg("connectivityLength"), py::arg("numberOfCells"), py::arg("numberOfPoints"))
    .def_static("determineAllocationLengths", (void (*)(::smtk::model::Loop const &, ::smtk::mesh::CollectionPtr const &, ::int64_t &, ::int64_t &, ::int64_t &)) &smtk::mesh::PreAllocatedTessellation::determineAllocationLengths, py::arg("loop"), py::arg("c"), py::arg("connectivityLength"), py::arg("numberOfCells"), py::arg("numberOfPoints"))
    .def("disableVTKCellTypes", &smtk::mesh::PreAllocatedTessellation::disableVTKCellTypes, py::arg("disable"))
    .def("disableVTKStyleConnectivity", &smtk::mesh::PreAllocatedTessellation::disableVTKStyleConnectivity, py::arg("disable"))
    .def("hasCellLocations", &smtk::mesh::PreAllocatedTessellation::hasCellLocations)
    .def("hasCellTypes", &smtk::mesh::PreAllocatedTessellation::hasCellTypes)
    .def("hasConnectivity", &smtk::mesh::PreAllocatedTessellation::hasConnectivity)
    .def("hasDoublePoints", &smtk::mesh::PreAllocatedTessellation::hasDoublePoints)
    .def("hasFloatPoints", &smtk::mesh::PreAllocatedTessellation::hasFloatPoints)
    .def("useVTKCellTypes", &smtk::mesh::PreAllocatedTessellation::useVTKCellTypes)
    .def("useVTKConnectivity", &smtk::mesh::PreAllocatedTessellation::useVTKConnectivity)
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::Tessellation > pybind11_init_smtk_mesh_Tessellation(py::module &m)
{
  PySharedPtrClass< smtk::mesh::Tessellation > instance(m, "Tessellation");
  instance
    .def(py::init<::smtk::mesh::Tessellation const &>())
    .def(py::init<>())
    .def(py::init<bool, bool>())
    .def("deepcopy", (smtk::mesh::Tessellation & (smtk::mesh::Tessellation::*)(::smtk::mesh::Tessellation const &)) &smtk::mesh::Tessellation::operator=)
    .def("cellLocations", &smtk::mesh::Tessellation::cellLocations)
    .def("cellTypes", &smtk::mesh::Tessellation::cellTypes)
    .def("connectivity", &smtk::mesh::Tessellation::connectivity)
    .def("extract", (void (smtk::mesh::Tessellation::*)(::smtk::mesh::MeshSet const &)) &smtk::mesh::Tessellation::extract, py::arg("ms"))
    .def("extract", (void (smtk::mesh::Tessellation::*)(::smtk::mesh::CellSet const &)) &smtk::mesh::Tessellation::extract, py::arg("cs"))
    .def("extract", (void (smtk::mesh::Tessellation::*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &)) &smtk::mesh::Tessellation::extract, py::arg("cs"), py::arg("ps"))
    .def("extract", (void (smtk::mesh::Tessellation::*)(::smtk::mesh::CellSet const &, ::smtk::mesh::PointSet const &)) &smtk::mesh::Tessellation::extract, py::arg("cs"), py::arg("ps"))
    .def("points", &smtk::mesh::Tessellation::points)
    .def("useVTKCellTypes", &smtk::mesh::Tessellation::useVTKCellTypes)
    .def("useVTKConnectivity", &smtk::mesh::Tessellation::useVTKConnectivity)
    ;
  return instance;
}

void pybind11_init__ZN4smtk4mesh26extractOrderedTessellationERKNS_5model4EdgeERKNSt3__110shared_ptrINS0_10CollectionEEERNS0_24PreAllocatedTessellationE(py::module &m)
{
  m.def("extractOrderedTessellation", (void (*)(::smtk::model::Edge const &, ::smtk::mesh::CollectionPtr const &, ::smtk::mesh::PreAllocatedTessellation &)) &smtk::mesh::extractOrderedTessellation, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

void pybind11_init__ZN4smtk4mesh26extractOrderedTessellationERKNS_5model4LoopERKNSt3__110shared_ptrINS0_10CollectionEEERNS0_24PreAllocatedTessellationE(py::module &m)
{
  m.def("extractOrderedTessellation", (void (*)(::smtk::model::Loop const &, ::smtk::mesh::CollectionPtr const &, ::smtk::mesh::PreAllocatedTessellation &)) &smtk::mesh::extractOrderedTessellation, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

void pybind11_init__ZN4smtk4mesh26extractOrderedTessellationERKNS_5model4EdgeERKNSt3__110shared_ptrINS0_10CollectionEEERKNS0_8PointSetERNS0_24PreAllocatedTessellationE(py::module &m)
{
  m.def("extractOrderedTessellation", (void (*)(::smtk::model::Edge const &, ::smtk::mesh::CollectionPtr const &, ::smtk::mesh::PointSet const &, ::smtk::mesh::PreAllocatedTessellation &)) &smtk::mesh::extractOrderedTessellation, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"), py::arg("arg3"));
}

void pybind11_init__ZN4smtk4mesh26extractOrderedTessellationERKNS_5model4LoopERKNSt3__110shared_ptrINS0_10CollectionEEERKNS0_8PointSetERNS0_24PreAllocatedTessellationE(py::module &m)
{
  m.def("extractOrderedTessellation", (void (*)(::smtk::model::Loop const &, ::smtk::mesh::CollectionPtr const &, ::smtk::mesh::PointSet const &, ::smtk::mesh::PreAllocatedTessellation &)) &smtk::mesh::extractOrderedTessellation, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"), py::arg("arg3"));
}

void pybind11_init__ZN4smtk4mesh19extractTessellationERKNS0_7MeshSetERNS0_24PreAllocatedTessellationE(py::module &m)
{
  m.def("extractTessellation", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PreAllocatedTessellation &)) &smtk::mesh::extractTessellation, "", py::arg("arg0"), py::arg("arg1"));
}

void pybind11_init__ZN4smtk4mesh19extractTessellationERKNS0_7CellSetERNS0_24PreAllocatedTessellationE(py::module &m)
{
  m.def("extractTessellation", (void (*)(::smtk::mesh::CellSet const &, ::smtk::mesh::PreAllocatedTessellation &)) &smtk::mesh::extractTessellation, "", py::arg("arg0"), py::arg("arg1"));
}

void pybind11_init__ZN4smtk4mesh19extractTessellationERKNS_5model9EntityRefERKNSt3__110shared_ptrINS0_10CollectionEEERNS0_24PreAllocatedTessellationE(py::module &m)
{
  m.def("extractTessellation", (void (*)(::smtk::model::EntityRef const &, ::smtk::mesh::CollectionPtr const &, ::smtk::mesh::PreAllocatedTessellation &)) &smtk::mesh::extractTessellation, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

void pybind11_init__ZN4smtk4mesh19extractTessellationERKNS0_7MeshSetERKNS0_8PointSetERNS0_24PreAllocatedTessellationE(py::module &m)
{
  m.def("extractTessellation", (void (*)(::smtk::mesh::MeshSet const &, ::smtk::mesh::PointSet const &, ::smtk::mesh::PreAllocatedTessellation &)) &smtk::mesh::extractTessellation, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

void pybind11_init__ZN4smtk4mesh19extractTessellationERKNS0_7CellSetERKNS0_8PointSetERNS0_24PreAllocatedTessellationE(py::module &m)
{
  m.def("extractTessellation", (void (*)(::smtk::mesh::CellSet const &, ::smtk::mesh::PointSet const &, ::smtk::mesh::PreAllocatedTessellation &)) &smtk::mesh::extractTessellation, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

void pybind11_init__ZN4smtk4mesh19extractTessellationERNS0_17PointConnectivityERKNS0_8PointSetERNS0_24PreAllocatedTessellationE(py::module &m)
{
  m.def("extractTessellation", (void (*)(::smtk::mesh::PointConnectivity &, ::smtk::mesh::PointSet const &, ::smtk::mesh::PreAllocatedTessellation &)) &smtk::mesh::extractTessellation, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"));
}

void pybind11_init__ZN4smtk4mesh19extractTessellationERKNS_5model9EntityRefERKNSt3__110shared_ptrINS0_10CollectionEEERKNS0_8PointSetERNS0_24PreAllocatedTessellationE(py::module &m)
{
  m.def("extractTessellation", (void (*)(::smtk::model::EntityRef const &, ::smtk::mesh::CollectionPtr const &, ::smtk::mesh::PointSet const &, ::smtk::mesh::PreAllocatedTessellation &)) &smtk::mesh::extractTessellation, "", py::arg("arg0"), py::arg("arg1"), py::arg("arg2"), py::arg("arg3"));
}

#endif
