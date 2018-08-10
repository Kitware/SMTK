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
#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <utility>

#include "vtkObject.h"
#include "vtkSmartPointer.h"

#include "smtk/extension/vtk/pybind11/PybindVTKTypeCaster.h"
#include "smtk/extension/vtk/source/vtkMeshMultiBlockSource.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/model/Resource.h"

#include "smtk/extension/vtk/source/PointCloudFromVTKAuxiliaryGeometry.h"
#include "smtk/extension/vtk/source/StructuredGridFromVTKAuxiliaryGeometry.h"

namespace py = pybind11;

PYBIND11_VTK_TYPECASTER(vtkModelMultiBlockSource)
PYBIND11_VTK_TYPECASTER(vtkMeshMultiBlockSource)

PYBIND11_MODULE(_smtkPybindVTKSourceFns, source)
{
  source.doc() = "<description>";

  source.def("_vtkModelMultiBlockSource_GetModelResource",[&](vtkModelMultiBlockSource* obj){ return obj->GetModelResource(); });
  source.def("_vtkModelMultiBlockSource_SetModelResource",[&](vtkModelMultiBlockSource* obj, smtk::model::ResourcePtr resource){ return obj->SetModelResource(resource); });

  source.def("_vtkMeshMultiBlockSource_GetModelResource",[&](vtkMeshMultiBlockSource* obj){ obj->GetModelResource(); });
  source.def("_vtkMeshMultiBlockSource_SetModelResource",[&](vtkMeshMultiBlockSource* obj, smtk::model::ResourcePtr resource){ return obj->SetModelResource(resource); });
  source.def("_vtkMeshMultiBlockSource_GetMeshManager",[&](vtkMeshMultiBlockSource* obj){ obj->GetMeshManager(); });
  source.def("_vtkMeshMultiBlockSource_SetMeshManager",[&](vtkMeshMultiBlockSource* obj, smtk::mesh::ManagerPtr manager){ return obj->SetMeshManager(manager); });

  bool pcRegistered =
    smtk::extension::vtk::mesh::PointCloudFromVTKAuxiliaryGeometry::registerClass();
  (void)pcRegistered;
  bool sgRegistered =
    smtk::extension::vtk::mesh::StructuredGridFromVTKAuxiliaryGeometry::registerClass();
  (void)sgRegistered;
}
