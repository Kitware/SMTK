//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_vtk_io_mesh_ImportSTLFile_h
#define pybind_smtk_extension_vtk_io_mesh_ImportSTLFile_h

#include <pybind11/pybind11.h>

#include "smtk/extension/vtk/io/mesh/ImportSTLFile.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"

namespace py = pybind11;

py::class_< smtk::extension::vtk::io::mesh::ImportSTLFile > pybind11_init_smtk_extension_vtk_io_mesh_ImportSTLFile(py::module &m)
{
  py::class_< smtk::extension::vtk::io::mesh::ImportSTLFile > instance(m, "ImportSTLFile");
  instance
    .def(py::init<>())
    .def("__call__", (smtk::mesh::CollectionPtr (smtk::extension::vtk::io::mesh::ImportSTLFile::*)(::std::string const &, ::smtk::mesh::ManagerPtr &) const) &smtk::extension::vtk::io::mesh::ImportSTLFile::operator())
    .def("__call__", (bool (smtk::extension::vtk::io::mesh::ImportSTLFile::*)(::std::string const &, ::smtk::mesh::CollectionPtr) const) &smtk::extension::vtk::io::mesh::ImportSTLFile::operator())
    ;
  return instance;
}

#endif
