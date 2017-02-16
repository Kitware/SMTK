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

#include "PybindRemusConnection.h"
#include "PybindRemusConnections.h"
// #include "PybindRemusRPCWorker.h"
#include "PybindRemusStaticSessionInfo.h"
#include "PybindSession.h"

#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_PLUGIN(_smtkPybindRemoteSession)
{
  py::module remote("_smtkPybindRemoteSession", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::bridge::remote::RemusConnection > smtk_bridge_remote_RemusConnection = pybind11_init_smtk_bridge_remote_RemusConnection(remote);
  py::class_< smtk::bridge::remote::RemusConnections > smtk_bridge_remote_RemusConnections = pybind11_init_smtk_bridge_remote_RemusConnections(remote);
  // py::class_< smtk::bridge::remote::RemusRPCWorker > smtk_bridge_remote_RemusRPCWorker = pybind11_init_smtk_bridge_remote_RemusRPCWorker(remote);
  py::class_< smtk::bridge::remote::RemusStaticSessionInfo > smtk_bridge_remote_RemusStaticSessionInfo = pybind11_init_smtk_bridge_remote_RemusStaticSessionInfo(remote);
  PySharedPtrClass< smtk::bridge::remote::Session, smtk::model::Session > smtk_bridge_remote_Session = pybind11_init_smtk_bridge_remote_Session(remote);

  return remote.ptr();
}
