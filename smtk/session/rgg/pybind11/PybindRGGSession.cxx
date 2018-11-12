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

#include "PybindAddMaterial.h"
#include "PybindCreateAssembly.h"
#include "PybindCreateDuct.h"
#include "PybindCreateModel.h"
#include "PybindCreatePin.h"
#include "PybindDelete.h"
#include "PybindEditAssembly.h"
#include "PybindEditCore.h"
#include "PybindEditDuct.h"
#include "PybindEditMaterial.h"
#include "PybindReadRXFFile.h"
#include "PybindRemoveMaterial.h"

#include "PybindRegistrar.h"

#include "smtk/model/Session.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindRGGSession, rgg)
{
  rgg.doc() = "<description>";

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::session::rgg::Session, smtk::model::Session > smtk_session_rgg_Session = pybind11_init_smtk_session_rgg_Session(rgg);
  PySharedPtrClass< smtk::session::rgg::Resource, smtk::model::Resource > smtk_session_rgg_Resource = pybind11_init_smtk_session_rgg_Resource(rgg);

  PySharedPtrClass< smtk::session::rgg::AddMaterial, smtk::operation::XMLOperation > smtk_session_rgg_AddMaterial = pybind11_init_smtk_session_rgg_AddMaterial(rgg);
  PySharedPtrClass< smtk::session::rgg::CreateAssembly, smtk::operation::XMLOperation > smtk_session_rgg_CreateAssembly = pybind11_init_smtk_session_rgg_CreateAssembly(rgg);
  PySharedPtrClass< smtk::session::rgg::CreateDuct, smtk::operation::XMLOperation > smtk_session_rgg_CreateDuct = pybind11_init_smtk_session_rgg_CreateDuct(rgg);
  PySharedPtrClass< smtk::session::rgg::CreateModel, smtk::operation::XMLOperation > smtk_session_rgg_CreateModel = pybind11_init_smtk_session_rgg_CreateModel(rgg);
  PySharedPtrClass< smtk::session::rgg::CreatePin, smtk::operation::XMLOperation > smtk_session_rgg_CreatePin = pybind11_init_smtk_session_rgg_CreatePin(rgg);
  PySharedPtrClass< smtk::session::rgg::Delete, smtk::operation::XMLOperation > smtk_session_rgg_Delete = pybind11_init_smtk_session_rgg_Delete(rgg);
  PySharedPtrClass< smtk::session::rgg::EditAssembly, smtk::operation::XMLOperation > smtk_session_rgg_EditAssembly = pybind11_init_smtk_session_rgg_EditAssembly(rgg);
  PySharedPtrClass< smtk::session::rgg::EditCore, smtk::operation::XMLOperation > smtk_session_rgg_EditCore = pybind11_init_smtk_session_rgg_EditCore(rgg);
  PySharedPtrClass< smtk::session::rgg::EditDuct, smtk::operation::XMLOperation > smtk_session_rgg_EditDuct = pybind11_init_smtk_session_rgg_EditDuct(rgg);
  PySharedPtrClass< smtk::session::rgg::EditMaterial, smtk::operation::XMLOperation > smtk_session_rgg_EditMaterial = pybind11_init_smtk_session_rgg_EditMaterial(rgg);
  PySharedPtrClass< smtk::session::rgg::ReadRXFFile, smtk::operation::XMLOperation > smtk_session_rgg_ReadRXFFile = pybind11_init_smtk_session_rgg_ReadRXFFile(rgg);
  PySharedPtrClass< smtk::session::rgg::RemoveMaterial, smtk::operation::XMLOperation > smtk_session_rgg_RemoveMaterial = pybind11_init_smtk_session_rgg_RemoveMaterial(rgg);

  py::class_< smtk::session::rgg::Registrar > smtk_session_rgg_Registrar = pybind11_init_smtk_session_rgg_Registrar(rgg);
}
