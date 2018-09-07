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

#include "PybindOperation.h"
#include "PybindPointerDefs.h"
#include "PybindExport.h"
#include "PybindImport.h"
#include "PybindLegacyRead.h"
#include "PybindRead.h"
#include "PybindSession.h"
#include "PybindWrite.h"

#include "PybindRegistrar.h"

#include "smtk/model/Session.h"
#include "smtk/model/SessionIOJSON.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindVTKSession, vtk)
{
  vtk.doc() = "<description>";

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::session::vtk::EntityHandle > smtk_session_vtk_EntityHandle = pybind11_init_smtk_session_vtk_EntityHandle(vtk);
  pybind11_init_smtk_session_vtk_EntityType(vtk);
  pybind11_init_smtk_session_vtk_EntityTypeNameString(vtk);
  PySharedPtrClass< smtk::session::vtk::Operation, smtk::operation::XMLOperation > smtk_session_vtk_Operation = pybind11_init_smtk_session_vtk_Operation(vtk);
  PySharedPtrClass< smtk::session::vtk::Session, smtk::model::Session > smtk_session_vtk_Session = pybind11_init_smtk_session_vtk_Session(vtk);
  PySharedPtrClass< smtk::session::vtk::Import > smtk_session_vtk_Import = pybind11_init_smtk_session_vtk_Import(vtk, smtk_session_vtk_Operation);
  PySharedPtrClass< smtk::session::vtk::Export > smtk_session_vtk_Export = pybind11_init_smtk_session_vtk_Export(vtk, smtk_session_vtk_Operation);
  PySharedPtrClass< smtk::session::vtk::LegacyRead > smtk_session_vtk_LegacyRead = pybind11_init_smtk_session_vtk_LegacyRead(vtk, smtk_session_vtk_Operation);
  PySharedPtrClass< smtk::session::vtk::Read > smtk_session_vtk_Read = pybind11_init_smtk_session_vtk_Read(vtk, smtk_session_vtk_Operation);
  PySharedPtrClass< smtk::session::vtk::Write > smtk_session_vtk_Write = pybind11_init_smtk_session_vtk_Write(vtk, smtk_session_vtk_Operation);

  py::class_< smtk::session::vtk::Registrar > smtk_session_vtk_Registrar = pybind11_init_smtk_session_vtk_Registrar(vtk);
}
