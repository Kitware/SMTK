//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_vtk_io_mesh_ExportVTKData_h
#define pybind_smtk_extension_vtk_io_mesh_ExportVTKData_h

#include <pybind11/pybind11.h>

#include "smtk/extension/vtk/io/mesh/ExportVTKData.h"

namespace py = pybind11;

py::class_< smtk::extension::vtk::io::mesh::ExportVTKData > pybind11_init_smtk_extension_vtk_io_mesh_ExportVTKData(py::module &m)
{
  py::class_< smtk::extension::vtk::io::mesh::ExportVTKData > instance(m, "ExportVTKData");
  instance
    .def(py::init<>())
    .def("__call__", (bool (smtk::extension::vtk::io::mesh::ExportVTKData::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::std::string) const) &smtk::extension::vtk::io::mesh::ExportVTKData::operator())
    .def("__call__", (void (smtk::extension::vtk::io::mesh::ExportVTKData::*)(::smtk::mesh::MeshSet const &, ::vtkPolyData *, ::std::string) const) &smtk::extension::vtk::io::mesh::ExportVTKData::operator(), py::arg("meshSet"), py::arg("polydata"), py::arg("domain"))
    .def("__call__", [&](const smtk::extension::vtk::io::mesh::ExportVTKData& exportData, ::smtk::mesh::MeshSet const & ms, ::vtkPolyData* pd){ return exportData(ms, pd); })
    .def("__call__", (void (smtk::extension::vtk::io::mesh::ExportVTKData::*)(::smtk::mesh::MeshSet const &, ::vtkUnstructuredGrid *, ::std::string) const) &smtk::extension::vtk::io::mesh::ExportVTKData::operator())
    .def("__call__", [&](const smtk::extension::vtk::io::mesh::ExportVTKData& exportData, ::smtk::mesh::MeshSet const & ms, ::vtkUnstructuredGrid* ug){ return exportData(ms, ug); })
    ;
  return instance;
}

#endif
