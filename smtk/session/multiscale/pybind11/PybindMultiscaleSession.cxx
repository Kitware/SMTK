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

#include "PybindPartitionBoundaries.h"
#include "PybindRevolve.h"
#include "PybindSession.h"

#include "PybindResource.h"
#include "PybindRegistrar.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindMultiscaleSession, multiscale)
{
  multiscale.doc() = "<description>";

  py::module::import("smtk.session.mesh");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::session::multiscale::Resource> smtk_session_multiscale_Resource = pybind11_init_smtk_session_multiscale_Resource(multiscale);
  PySharedPtrClass< smtk::session::multiscale::Session, smtk::session::mesh::Session > smtk_session_multiscale_Session = pybind11_init_smtk_session_multiscale_Session(multiscale);

  PySharedPtrClass< smtk::session::multiscale::Revolve, smtk::operation::XMLOperation > smtk_session_multiscale_Revolve = pybind11_init_smtk_session_multiscale_Revolve(multiscale);
  PySharedPtrClass< smtk::session::multiscale::PartitionBoundaries, smtk::operation::XMLOperation > smtk_session_multiscale_PartitionBoundaries = pybind11_init_smtk_session_multiscale_PartitionBoundaries(multiscale);

  py::class_< smtk::session::multiscale::Registrar > smtk_session_multiscale_Registrar = pybind11_init_smtk_session_multiscale_Registrar(multiscale);
}
