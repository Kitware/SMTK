//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_vtk_io_ImportAsVTKData_h
#define pybind_smtk_extension_vtk_io_ImportAsVTKData_h

#include <pybind11/pybind11.h>

#include "smtk/extension/vtk/io/IOVTKExports.h"

#include "vtkSmartPointer.h"

#include "smtk/extension/vtk/io/ImportAsVTKData.h"

namespace py = pybind11;

inline py::class_<smtk::extension::vtk::io::ImportAsVTKData> pybind11_init_smtk_extension_vtk_io_ImportAsVTKData(py::module &m)
{
  py::class_< smtk::extension::vtk::io::ImportAsVTKData> instance(m, "ImportAsVTKData");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::extension::vtk::io::ImportAsVTKData const &>())
    .def("__call__", (vtkSmartPointer<vtkDataObject> (smtk::extension::vtk::io::ImportAsVTKData::*)(::std::string const &)) &smtk::extension::vtk::io::ImportAsVTKData::operator())
    .def("deepcopy", (smtk::extension::vtk::io::ImportAsVTKData & (smtk::extension::vtk::io::ImportAsVTKData::*)(::smtk::extension::vtk::io::ImportAsVTKData const &)) &smtk::extension::vtk::io::ImportAsVTKData::operator=)
    .def("valid", (bool (smtk::extension::vtk::io::ImportAsVTKData::*)(::std::string const&) const) &smtk::extension::vtk::io::ImportAsVTKData::valid, py::arg("file"))
    ;
  return instance;
}

#endif
