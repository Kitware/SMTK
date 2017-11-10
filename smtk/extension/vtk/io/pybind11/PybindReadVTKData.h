//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_vtk_io_ReadVTKData_h
#define pybind_smtk_extension_vtk_io_ReadVTKData_h

#include <pybind11/pybind11.h>

#include "smtk/extension/vtk/io/IOVTKExports.h"

#include "vtkSmartPointer.h"

#include "smtk/extension/vtk/io/ReadVTKData.h"

namespace py = pybind11;

py::class_<smtk::extension::vtk::io::ReadVTKData> pybind11_init_smtk_extension_vtk_io_ReadVTKData(py::module &m)
{
  py::class_< smtk::extension::vtk::io::ReadVTKData> instance(m, "ReadVTKData");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::extension::vtk::io::ReadVTKData const &>())
    .def("__call__", (vtkSmartPointer<vtkDataObject> (smtk::extension::vtk::io::ReadVTKData::*)(::std::string const &)) &smtk::extension::vtk::io::ReadVTKData::operator())
    .def("deepcopy", (smtk::extension::vtk::io::ReadVTKData & (smtk::extension::vtk::io::ReadVTKData::*)(::smtk::extension::vtk::io::ReadVTKData const &)) &smtk::extension::vtk::io::ReadVTKData::operator=)
    .def("valid", (bool (smtk::extension::vtk::io::ReadVTKData::*)(::std::string const&) const) &smtk::extension::vtk::io::ReadVTKData::valid, py::arg("file"))
    ;
  return instance;
}

#endif
