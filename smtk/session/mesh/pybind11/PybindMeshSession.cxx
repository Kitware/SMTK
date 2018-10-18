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

#include "PybindResource.h"
#include "PybindSession.h"
#include "PybindTopology.h"

#include "PybindImport.h"
#include "PybindExport.h"
#include "PybindRead.h"
#include "PybindEulerCharacteristicRatio.h"
#include "PybindWrite.h"

#include "PybindRegistrar.h"

#include "smtk/model/Session.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindMeshSession, mesh)
{
  mesh.doc() = "<description>";

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::session::mesh::Topology > smtk_session_mesh_Topology = pybind11_init_smtk_session_mesh_Topology(mesh);
  PySharedPtrClass< smtk::session::mesh::Session, smtk::model::Session > smtk_session_mesh_Session = pybind11_init_smtk_session_mesh_Session(mesh);
  PySharedPtrClass< smtk::session::mesh::Resource, smtk::model::Resource > smtk_session_mesh_Resource = pybind11_init_smtk_session_mesh_Resource(mesh);

  PySharedPtrClass< smtk::session::mesh::EulerCharacteristicRatio, smtk::operation::XMLOperation > smtk_session_mesh_EulerCharacteristicRatio = pybind11_init_smtk_session_mesh_EulerCharacteristicRatio(mesh);
  PySharedPtrClass< smtk::session::mesh::Import, smtk::operation::XMLOperation > smtk_session_mesh_Import = pybind11_init_smtk_session_mesh_Import(mesh);
  PySharedPtrClass< smtk::session::mesh::Export, smtk::operation::XMLOperation > smtk_session_mesh_Export = pybind11_init_smtk_session_mesh_Export(mesh);
  PySharedPtrClass< smtk::session::mesh::Read, smtk::operation::XMLOperation > smtk_session_mesh_Read = pybind11_init_smtk_session_mesh_Read(mesh);
  PySharedPtrClass< smtk::session::mesh::Write, smtk::operation::XMLOperation > smtk_session_mesh_Write = pybind11_init_smtk_session_mesh_Write(mesh);

  py::class_< smtk::session::mesh::Registrar > smtk_session_mesh_Registrar = pybind11_init_smtk_session_mesh_Registrar(mesh);
}
