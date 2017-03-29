//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_CellTypes_h
#define pybind_smtk_mesh_CellTypes_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/CellTraits.h"
#include "smtk/mesh/CellTypes.h"

namespace py = pybind11;

void pybind11_init_smtk_mesh_CellType(py::module &m)
{
  py::enum_<smtk::mesh::CellType>(m, "CellType")
    .value("Vertex", smtk::mesh::CellType::Vertex)
    .value("Line", smtk::mesh::CellType::Line)
    .value("Triangle", smtk::mesh::CellType::Triangle)
    .value("Quad", smtk::mesh::CellType::Quad)
    .value("Polygon", smtk::mesh::CellType::Polygon)
    .value("Tetrahedron", smtk::mesh::CellType::Tetrahedron)
    .value("Pyramid", smtk::mesh::CellType::Pyramid)
    .value("Wedge", smtk::mesh::CellType::Wedge)
    .value("Hexahedron", smtk::mesh::CellType::Hexahedron)
    .value("CellType_MAX", smtk::mesh::CellType::CellType_MAX)
    .export_values();
}

void pybind11_init_smtk_mesh_verticesPerCell(py::module &m)
{
  m.def("verticesPerCell", &smtk::mesh::verticesPerCell, "", py::arg("ctype"));
}

void pybind11_init_smtk_mesh_cellTypeSummary(py::module &m)
{
  m.def("cellTypeSummary", &smtk::mesh::cellTypeSummary, "", py::arg("ctype"), py::arg("flag") = 0);
}

#endif
