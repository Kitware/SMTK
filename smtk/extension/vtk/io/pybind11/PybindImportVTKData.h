//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_vtk_io_mesh_ImportVTKData_h
#define pybind_smtk_extension_vtk_io_mesh_ImportVTKData_h

#include <pybind11/pybind11.h>

#include "smtk/extension/vtk/io/mesh/ImportVTKData.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

namespace py = pybind11;

py::class_< smtk::extension::vtk::io::mesh::ImportVTKData > pybind11_init_smtk_extension_vtk_io_mesh_ImportVTKData(py::module &m)
{
  py::class_< smtk::extension::vtk::io::mesh::ImportVTKData > instance(m, "ImportVTKData");
  instance
    .def(py::init<>())
    .def("__call__", (smtk::mesh::CollectionPtr (smtk::extension::vtk::io::mesh::ImportVTKData::*)(::std::string const &, ::smtk::mesh::ManagerPtr &, ::std::string) const) &smtk::extension::vtk::io::mesh::ImportVTKData::operator())
    .def("__call__", (bool (smtk::extension::vtk::io::mesh::ImportVTKData::*)(::std::string const &, ::smtk::mesh::CollectionPtr, ::std::string) const) &smtk::extension::vtk::io::mesh::ImportVTKData::operator())
    .def("__call__", (smtk::mesh::MeshSet (smtk::extension::vtk::io::mesh::ImportVTKData::*)(::vtkPolyData *, ::smtk::mesh::CollectionPtr) const) &smtk::extension::vtk::io::mesh::ImportVTKData::operator())
    .def("__call__", (bool (smtk::extension::vtk::io::mesh::ImportVTKData::*)(::vtkPolyData *, ::smtk::mesh::CollectionPtr, ::std::string) const) &smtk::extension::vtk::io::mesh::ImportVTKData::operator())
    .def("__call__", (smtk::mesh::CollectionPtr (smtk::extension::vtk::io::mesh::ImportVTKData::*)(::vtkPolyData *, ::smtk::mesh::ManagerPtr &, ::std::string) const) &smtk::extension::vtk::io::mesh::ImportVTKData::operator())
    .def("__call__", [&](const smtk::extension::vtk::io::mesh::ImportVTKData& importData, vtkPolyData *pd, ::smtk::mesh::ManagerPtr & manager){ return importData(pd, manager); })
    .def("__call__", (bool (smtk::extension::vtk::io::mesh::ImportVTKData::*)(::vtkUnstructuredGrid *, ::smtk::mesh::CollectionPtr, ::std::string) const) &smtk::extension::vtk::io::mesh::ImportVTKData::operator())
    .def("__call__", (smtk::mesh::CollectionPtr (smtk::extension::vtk::io::mesh::ImportVTKData::*)(::vtkUnstructuredGrid *, ::smtk::mesh::ManagerPtr &, ::std::string) const) &smtk::extension::vtk::io::mesh::ImportVTKData::operator())
    .def("__call__", [&](const smtk::extension::vtk::io::mesh::ImportVTKData& importData, vtkUnstructuredGrid *ug, ::smtk::mesh::ManagerPtr & manager){ return importData(ug, manager); })
    ;
  return instance;
}

#endif
