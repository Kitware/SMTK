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

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindOperator.h"
#include "PybindPointerDefs.h"
#include "PybindSession.h"
#include "PybindTopology.h"

#include "PybindImportOperator.h"
#include "PybindReadOperator.h"
#include "PybindWriteOperator.h"

#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_PLUGIN(_smtkPybindMeshSession)
{
  py::module mesh("_smtkPybindMeshSession", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::bridge::mesh::Topology > smtk_bridge_mesh_Topology = pybind11_init_smtk_bridge_mesh_Topology(mesh);
  PySharedPtrClass< smtk::bridge::mesh::Operator, smtk::model::Operator > smtk_bridge_mesh_Operator = pybind11_init_smtk_bridge_mesh_Operator(mesh);
  PySharedPtrClass< smtk::bridge::mesh::Session, smtk::model::Session > smtk_bridge_mesh_Session = pybind11_init_smtk_bridge_mesh_Session(mesh);

  PySharedPtrClass< smtk::bridge::mesh::ImportOperator > smtk_bridge_mesh_ImportOperator = pybind11_init_smtk_bridge_mesh_ImportOperator(mesh, smtk_bridge_mesh_Operator);
  PySharedPtrClass< smtk::bridge::mesh::ReadOperator > smtk_bridge_mesh_ReadOperator = pybind11_init_smtk_bridge_mesh_ReadOperator(mesh, smtk_bridge_mesh_Operator);
  PySharedPtrClass< smtk::bridge::mesh::WriteOperator > smtk_bridge_mesh_WriteOperator = pybind11_init_smtk_bridge_mesh_WriteOperator(mesh, smtk_bridge_mesh_Operator);

  return mesh.ptr();
}
