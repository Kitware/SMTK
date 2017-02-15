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
#include "PybindReadOperator.h"
#include "PybindSession.h"
#include "PybindSessionExodusIOJSON.h"
#include "PybindWriteOperator.h"

#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"
#include "smtk/model/SessionIOJSON.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_PLUGIN(_smtkPybindExodusSession)
{
  py::module exodus("_smtkPybindExodusSession", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::bridge::exodus::EntityHandle > smtk_bridge_exodus_EntityHandle = pybind11_init_smtk_bridge_exodus_EntityHandle(exodus);
  pybind11_init_smtk_bridge_exodus_EntityType(exodus);
  pybind11_init_smtk_bridge_exodus_EntityTypeNameString(exodus);
  PySharedPtrClass< smtk::bridge::exodus::Operator, smtk::model::Operator > smtk_bridge_exodus_Operator = pybind11_init_smtk_bridge_exodus_Operator(exodus);
  PySharedPtrClass< smtk::bridge::exodus::Session, smtk::model::Session > smtk_bridge_exodus_Session = pybind11_init_smtk_bridge_exodus_Session(exodus);
  py::class_< smtk::bridge::exodus::SessionIOJSON, smtk::model::SessionIOJSON > smtk_bridge_exodus_SessionIOJSON = pybind11_init_smtk_bridge_exodus_SessionIOJSON(exodus);
  PySharedPtrClass< smtk::bridge::exodus::ReadOperator > smtk_bridge_exodus_ReadOperator = pybind11_init_smtk_bridge_exodus_ReadOperator(exodus, smtk_bridge_exodus_Operator);
  PySharedPtrClass< smtk::bridge::exodus::WriteOperator > smtk_bridge_exodus_WriteOperator = pybind11_init_smtk_bridge_exodus_WriteOperator(exodus, smtk_bridge_exodus_Operator);

  return exodus.ptr();
}
