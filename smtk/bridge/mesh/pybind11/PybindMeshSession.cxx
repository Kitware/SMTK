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

#include "PybindSession.h"
#include "PybindTopology.h"

#include "PybindImportOperator.h"
#include "PybindExportOperator.h"
#include "PybindEulerCharacteristicRatio.h"

#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindMeshSession, mesh)
{
  mesh.doc() = "<description>";

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::bridge::mesh::Topology > smtk_bridge_mesh_Topology = pybind11_init_smtk_bridge_mesh_Topology(mesh);
  PySharedPtrClass< smtk::bridge::mesh::Session, smtk::model::Session > smtk_bridge_mesh_Session = pybind11_init_smtk_bridge_mesh_Session(mesh);

  PySharedPtrClass< smtk::bridge::mesh::EulerCharacteristicRatio, smtk::operation::XMLOperator > smtk_bridge_mesh_EulerCharacteristicRatio = pybind11_init_smtk_bridge_mesh_EulerCharacteristicRatio(mesh);
  PySharedPtrClass< smtk::bridge::mesh::ImportOperator, smtk::operation::XMLOperator > smtk_bridge_mesh_ImportOperator = pybind11_init_smtk_bridge_mesh_ImportOperator(mesh);
  PySharedPtrClass< smtk::bridge::mesh::ExportOperator, smtk::operation::XMLOperator > smtk_bridge_mesh_ExportOperator = pybind11_init_smtk_bridge_mesh_ExportOperator(mesh);
}
