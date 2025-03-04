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

inline py::class_< smtk::extension::vtk::io::mesh::ExportVTKData > pybind11_init_smtk_extension_vtk_io_mesh_ExportVTKData(py::module &m)
{
  py::class_< smtk::extension::vtk::io::mesh::ExportVTKData > instance(m, "ExportVTKData");
  instance
    .def(py::init<>())
    .def("__call__", (bool (smtk::extension::vtk::io::mesh::ExportVTKData::*)(::std::string const &, ::smtk::mesh::ResourcePtr, ::std::string) const) &smtk::extension::vtk::io::mesh::ExportVTKData::operator(), py::arg("filename"), py::arg("resource"), py::arg("domainPropertyName") = "")
    .def("__call__", (bool (smtk::extension::vtk::io::mesh::ExportVTKData::*)(::std::string const &, ::smtk::mesh::MeshSet const &, ::std::string) const) &smtk::extension::vtk::io::mesh::ExportVTKData::operator(), py::arg("filename"), py::arg("meshset"), py::arg("domainPropertyName") = "")
    .def("__call__", [&](const smtk::extension::vtk::io::mesh::ExportVTKData& exportData, ::smtk::mesh::MeshSet const & ms, ::vtkDataSet* ds){ if (auto *pd = vtkPolyData::SafeDownCast(ds)){ exportData(ms, pd);} else if (auto *ug = vtkUnstructuredGrid::SafeDownCast(ds)){ exportData(ms, ug);}})
    .def("__call__", [&](const smtk::extension::vtk::io::mesh::ExportVTKData& exportData, ::smtk::mesh::MeshSet const & ms, ::vtkDataSet* ds, const std::string& domain){ if (auto *pd = vtkPolyData::SafeDownCast(ds)){ exportData(ms, pd, domain);} else if (auto *ug = vtkUnstructuredGrid::SafeDownCast(ds)){ exportData(ms, ug, domain);}})
    ;
  return instance;
}

#endif
