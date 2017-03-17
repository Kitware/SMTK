//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <pybind11/pybind11.h>
#include <utility>

#include "vtkObject.h"
#include "vtkSmartPointer.h"

#include "smtk/extension/vtk/pybind11/PybindVTKTypeCaster.h"
#include "smtk/extension/vtk/source/vtkMeshMultiBlockSource.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "smtk/mesh/Manager.h"
#include "smtk/model/Manager.h"

namespace py = pybind11;

PYBIND11_VTK_TYPECASTER(vtkModelMultiBlockSource)
PYBIND11_VTK_TYPECASTER(vtkMeshMultiBlockSource)

PYBIND11_PLUGIN(_smtkPybindVTKSourceFns)
{
  py::module source("_smtkPybindVTKSourceFns", "<description>");

  source.def("_vtkModelMultiBlockSource_GetModelManager",[&](vtkModelMultiBlockSource* obj){ return obj->GetModelManager(); });
  source.def("_vtkModelMultiBlockSource_SetModelManager",[&](vtkModelMultiBlockSource* obj, smtk::model::ManagerPtr manager){ return obj->SetModelManager(manager); });

  source.def("_vtkMeshMultiBlockSource_GetModelManager",[&](vtkMeshMultiBlockSource* obj){ obj->GetModelManager(); });
  source.def("_vtkMeshMultiBlockSource_SetModelManager",[&](vtkMeshMultiBlockSource* obj, smtk::model::ManagerPtr manager){ return obj->SetModelManager(manager); });
  source.def("_vtkMeshMultiBlockSource_GetMeshManager",[&](vtkMeshMultiBlockSource* obj){ obj->GetMeshManager(); });
  source.def("_vtkMeshMultiBlockSource_SetMeshManager",[&](vtkMeshMultiBlockSource* obj, smtk::mesh::ManagerPtr manager){ return obj->SetMeshManager(manager); });

  return source.ptr();
}
