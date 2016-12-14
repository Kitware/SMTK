//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_vtk_io_ExportVTKData_h
#define pybind_smtk_extension_vtk_io_ExportVTKData_h

#include <pybind11/pybind11.h>

#include "smtk/extension/vtk/io/ExportVTKData.h"

// #include "vtkPolyData.h"
// #include "vtkUnstructuredGrid.h"

namespace py = pybind11;

py::class_< smtk::extension::vtk::io::ExportVTKData > pybind11_init_smtk_extension_vtk_io_ExportVTKData(py::module &m)
{
  py::class_< smtk::extension::vtk::io::ExportVTKData > instance(m, "ExportVTKData");
  instance
    .def(py::init<>())
    .def("__call__", (bool (smtk::extension::vtk::io::ExportVTKData::*)(::std::string const &, ::smtk::mesh::CollectionPtr, ::std::string) const) &smtk::extension::vtk::io::ExportVTKData::operator())
    // .def("__call__", (void (smtk::extension::vtk::io::ExportVTKData::*)(::smtk::mesh::MeshSet const &, ::vtkPolyData *, ::std::string) const) &smtk::extension::vtk::io::ExportVTKData::operator())
    // .def("__call__", (void (smtk::extension::vtk::io::ExportVTKData::*)(::smtk::mesh::MeshSet const &, ::vtkUnstructuredGrid *, ::std::string) const) &smtk::extension::vtk::io::ExportVTKData::operator())
    ;
  return instance;
}

#endif
