//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Tessellation_h
#define pybind_smtk_model_Tessellation_h

#include <pybind11/pybind11.h>

#include "smtk/model/Tessellation.h"

namespace py = pybind11;

void pybind11_init_smtk_model_TessellationCellType(py::module &m)
{
  py::enum_<smtk::model::TessellationCellType>(m, "TessellationCellType")
    .value("TESS_VERTEX", smtk::model::TessellationCellType::TESS_VERTEX)
    .value("TESS_TRIANGLE", smtk::model::TessellationCellType::TESS_TRIANGLE)
    .value("TESS_QUAD", smtk::model::TessellationCellType::TESS_QUAD)
    .value("TESS_POLYVERTEX", smtk::model::TessellationCellType::TESS_POLYVERTEX)
    .value("TESS_POLYLINE", smtk::model::TessellationCellType::TESS_POLYLINE)
    .value("TESS_POLYGON", smtk::model::TessellationCellType::TESS_POLYGON)
    .value("TESS_TRIANGLE_STRIP", smtk::model::TessellationCellType::TESS_TRIANGLE_STRIP)
    .value("TESS_INVALID_CELL", smtk::model::TessellationCellType::TESS_INVALID_CELL)
    .value("TESS_VARYING_VERT_CELL", smtk::model::TessellationCellType::TESS_VARYING_VERT_CELL)
    .value("TESS_CELLTYPE_MASK", smtk::model::TessellationCellType::TESS_CELLTYPE_MASK)
    .value("TESS_FACE_MATERIAL", smtk::model::TessellationCellType::TESS_FACE_MATERIAL)
    .value("TESS_FACE_UV", smtk::model::TessellationCellType::TESS_FACE_UV)
    .value("TESS_FACE_VERTEX_UV", smtk::model::TessellationCellType::TESS_FACE_VERTEX_UV)
    .value("TESS_FACE_NORMAL", smtk::model::TessellationCellType::TESS_FACE_NORMAL)
    .value("TESS_FACE_VERTEX_NORMAL", smtk::model::TessellationCellType::TESS_FACE_VERTEX_NORMAL)
    .value("TESS_FACE_COLOR", smtk::model::TessellationCellType::TESS_FACE_COLOR)
    .value("TESS_FACE_VERTEX_COLOR", smtk::model::TessellationCellType::TESS_FACE_VERTEX_COLOR)
    .value("TESS_PROPERTY_MASK", smtk::model::TessellationCellType::TESS_PROPERTY_MASK)
    .export_values();
}

py::class_< smtk::model::Tessellation > pybind11_init_smtk_model_Tessellation(py::module &m)
{
  py::class_< smtk::model::Tessellation > instance(m, "Tessellation");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::Tessellation const &>())
    .def("deepcopy", (smtk::model::Tessellation & (smtk::model::Tessellation::*)(::smtk::model::Tessellation const &)) &smtk::model::Tessellation::operator=)
    .def("coords", (std::vector<double, std::allocator<double> > & (smtk::model::Tessellation::*)()) &smtk::model::Tessellation::coords)
    .def("coords", (std::vector<double, std::allocator<double> > const & (smtk::model::Tessellation::*)() const) &smtk::model::Tessellation::coords)
    .def("conn", (std::vector<int, std::allocator<int> > & (smtk::model::Tessellation::*)()) &smtk::model::Tessellation::conn)
    .def("conn", (std::vector<int, std::allocator<int> > const & (smtk::model::Tessellation::*)() const) &smtk::model::Tessellation::conn)
    .def("addCoords", (int (smtk::model::Tessellation::*)(double *)) &smtk::model::Tessellation::addCoords, py::arg("a"))
    .def("addCoords", (smtk::model::Tessellation & (smtk::model::Tessellation::*)(double, double, double)) &smtk::model::Tessellation::addCoords, py::arg("x"), py::arg("y"), py::arg("z"))
    .def("addPoint", (smtk::model::Tessellation & (smtk::model::Tessellation::*)(double *)) &smtk::model::Tessellation::addPoint, py::arg("a"))
    .def("addLine", (smtk::model::Tessellation & (smtk::model::Tessellation::*)(double *, double *)) &smtk::model::Tessellation::addLine, py::arg("a"), py::arg("b"))
    .def("addTriangle", (smtk::model::Tessellation & (smtk::model::Tessellation::*)(double *, double *, double *)) &smtk::model::Tessellation::addTriangle, py::arg("a"), py::arg("b"), py::arg("c"))
    .def("addQuad", (smtk::model::Tessellation & (smtk::model::Tessellation::*)(double *, double *, double *, double *)) &smtk::model::Tessellation::addQuad, py::arg("a"), py::arg("b"), py::arg("c"), py::arg("d"))
    .def("addPoint", (smtk::model::Tessellation & (smtk::model::Tessellation::*)(int)) &smtk::model::Tessellation::addPoint, py::arg("ai"))
    .def("addLine", (smtk::model::Tessellation & (smtk::model::Tessellation::*)(int, int)) &smtk::model::Tessellation::addLine, py::arg("ai"), py::arg("bi"))
    .def("addTriangle", (smtk::model::Tessellation & (smtk::model::Tessellation::*)(int, int, int)) &smtk::model::Tessellation::addTriangle, py::arg("ai"), py::arg("bi"), py::arg("ci"))
    .def("addQuad", (smtk::model::Tessellation & (smtk::model::Tessellation::*)(int, int, int, int)) &smtk::model::Tessellation::addQuad, py::arg("ai"), py::arg("bi"), py::arg("ci"), py::arg("di"))
    .def("setPoint", &smtk::model::Tessellation::setPoint, py::arg("id"), py::arg("points"))
    .def("reset", &smtk::model::Tessellation::reset)
    .def("begin", &smtk::model::Tessellation::begin)
    .def("end", &smtk::model::Tessellation::end)
    .def("nextCellOffset", &smtk::model::Tessellation::nextCellOffset, py::arg("curOffset"))
    .def("cellType", &smtk::model::Tessellation::cellType, py::arg("offset"))
    .def("numberOfCellVertices", &smtk::model::Tessellation::numberOfCellVertices, py::arg("offset"), py::arg("cellTypeOut"))
    .def("vertexIdsOfCell", &smtk::model::Tessellation::vertexIdsOfCell, py::arg("offset"), py::arg("cellConn"))
    .def("materialIdOfCell", &smtk::model::Tessellation::materialIdOfCell, py::arg("offset"))
    .def("vertexIdsOfPolylineEndpoints", &smtk::model::Tessellation::vertexIdsOfPolylineEndpoints, py::arg("offset"), py::arg("first"), py::arg("last"))
    .def("insertNextCell", (smtk::model::Tessellation::size_type (smtk::model::Tessellation::*)(::std::vector<int, std::allocator<int> > &)) &smtk::model::Tessellation::insertNextCell, py::arg("cellConn"))
    .def("insertNextCell", (smtk::model::Tessellation::size_type (smtk::model::Tessellation::*)(::smtk::model::Tessellation::size_type, int const *)) &smtk::model::Tessellation::insertNextCell, py::arg("connLen"), py::arg("cellConn"))
    .def("insertCell", (bool (smtk::model::Tessellation::*)(::smtk::model::Tessellation::size_type, ::std::vector<int, std::allocator<int> > &)) &smtk::model::Tessellation::insertCell, py::arg("offset"), py::arg("cellConn"))
    .def("insertCell", (bool (smtk::model::Tessellation::*)(::smtk::model::Tessellation::size_type, ::smtk::model::Tessellation::size_type, int const *)) &smtk::model::Tessellation::insertCell, py::arg("offset"), py::arg("connLen"), py::arg("cellConn"))
    .def_static("cellShapeFromType", &smtk::model::Tessellation::cellShapeFromType, py::arg("arg0"))
    .def_static("numCellPropsFromType", &smtk::model::Tessellation::numCellPropsFromType, py::arg("cellType"))
    .def_static("numVertexPropsFromType", &smtk::model::Tessellation::numVertexPropsFromType, py::arg("cellType"))
    ;
  return instance;
}

#endif
