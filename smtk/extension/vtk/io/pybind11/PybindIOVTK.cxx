//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"

#include "PybindExportVTKData.h"
#include "PybindImportVTKData.h"
#include "PybindMeshIOVTK.h"
#include "PybindReadVTKData.h"

#include "smtk/io/mesh/MeshIO.h"

#include "smtk/extension/vtk/io/mesh/PointCloudFromVTKFile.h"
#include "smtk/extension/vtk/io/mesh/StructuredGridFromVTKFile.h"

#include "smtk/extension/vtk/pybind11/PybindVTKTypeCaster.h"

PYBIND11_VTK_TYPECASTER(vtkPolyData)
PYBIND11_VTK_TYPECASTER(vtkUnstructuredGrid)

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindIOVTK, io)
{
  io.doc() = "<description>";

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::extension::vtk::io::mesh::ExportVTKData > smtk_extension_vtk_io_mesh_ExportVTKData = pybind11_init_smtk_extension_vtk_io_mesh_ExportVTKData(io);
  py::class_< smtk::extension::vtk::io::mesh::ImportVTKData > smtk_extension_vtk_io_mesh_ImportVTKData = pybind11_init_smtk_extension_vtk_io_mesh_ImportVTKData(io);
  PySharedPtrClass< smtk::extension::vtk::io::mesh::MeshIOVTK, smtk::io::mesh::MeshIO > smtk_extension_vtk_io_mesh_MeshIOVTK = pybind11_init_smtk_extension_vtk_io_mesh_MeshIOVTK(io);
  py::class_<smtk::extension::vtk::io::ReadVTKData> smtk_extension_vtk_io_ReadVTKData = pybind11_init_smtk_extension_vtk_io_ReadVTKData(io);

  bool pcRegistered = smtk::extension::vtk::mesh::PointCloudFromVTKFile::registerClass();
  (void)pcRegistered;
  bool sgRegistered = smtk::extension::vtk::mesh::StructuredGridFromVTKFile::registerClass();
  (void)sgRegistered;
}
