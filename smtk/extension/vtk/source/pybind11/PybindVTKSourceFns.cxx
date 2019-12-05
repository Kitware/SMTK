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

#include "vtkInformation.h"
#include "vtkObject.h"
#include "vtkSmartPointer.h"

#include "smtk/extension/vtk/pybind11/PybindVTKTypeCaster.h"
#include "smtk/extension/vtk/source/vtkMeshMultiBlockSource.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Resource.h"

#include "smtk/extension/vtk/source/PointCloudFromVTKAuxiliaryGeometry.h"
#include "smtk/extension/vtk/source/StructuredGridFromVTKAuxiliaryGeometry.h"

namespace py = pybind11;

PYBIND11_VTK_TYPECASTER(vtkInformation)
PYBIND11_VTK_TYPECASTER(vtkModelMultiBlockSource)
PYBIND11_VTK_TYPECASTER(vtkMeshMultiBlockSource)

PYBIND11_MODULE(_smtkPybindVTKSourceFns, source)
{
  source.doc() = "<description>";

  source.def("_vtkModelMultiBlockSource_GetDataObjectUUID",[&](vtkInformation* info){ return vtkModelMultiBlockSource::GetDataObjectUUID(info); });
  source.def("_vtkModelMultiBlockSource_SetDataObjectUUID",[&](vtkInformation* info, const smtk::common::UUID& id){ return vtkModelMultiBlockSource::SetDataObjectUUID(info, id); });
  source.def("_vtkModelMultiBlockSource_GetComponent",[&](vtkModelMultiBlockSource* obj, vtkInformation* info){ return std::dynamic_pointer_cast<smtk::model::Entity>(obj->GetComponent(info)); });
  source.def("_vtkModelMultiBlockSource_GetModelResource",[&](vtkModelMultiBlockSource* obj){ return obj->GetModelResource(); });
  source.def("_vtkModelMultiBlockSource_SetModelResource",[&](vtkModelMultiBlockSource* obj, smtk::model::ResourcePtr resource){ return obj->SetModelResource(resource); });

  source.def("_vtkMeshMultiBlockSource_GetDataObjectUUID",[&](vtkInformation* info){ return vtkMeshMultiBlockSource::GetDataObjectUUID(info); });
  source.def("_vtkMeshMultiBlockSource_SetDataObjectUUID",[&](vtkInformation* info, const smtk::common::UUID& id){ return vtkMeshMultiBlockSource::SetDataObjectUUID(info, id); });
  source.def("_vtkMeshMultiBlockSource_GetComponent",[&](vtkMeshMultiBlockSource* obj, vtkInformation* info){ return std::dynamic_pointer_cast<smtk::mesh::Component>(obj->GetComponent(info)); });
  source.def("_vtkMeshMultiBlockSource_GetMeshResource",[&](vtkMeshMultiBlockSource* obj){ obj->GetMeshResource(); });
  source.def("_vtkMeshMultiBlockSource_SetMeshResource",[&](vtkMeshMultiBlockSource* obj, smtk::mesh::ResourcePtr resource){ return obj->SetMeshResource(resource); });

  bool pcRegistered =
    smtk::extension::vtk::mesh::PointCloudFromVTKAuxiliaryGeometry::registerClass();
  (void)pcRegistered;
  bool sgRegistered =
    smtk::extension::vtk::mesh::StructuredGridFromVTKAuxiliaryGeometry::registerClass();
  (void)sgRegistered;
}
