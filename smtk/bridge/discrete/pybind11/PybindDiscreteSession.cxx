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

#include "PybindArrangementHelper.h"
#include "PybindSession.h"
#include "PybindSession_json.h"

#include "smtk/mesh/core/ForEachTypes.h"

#include "smtk/model/ArrangementHelper.h"
#include "smtk/model/Session.h"

#include "PybindRegistrar.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindDiscreteSession, discrete)
{
  discrete.doc() = "<description>";

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::bridge::discrete::ArrangementHelper, smtk::model::ArrangementHelper > smtk_bridge_discrete_ArrangementHelper = pybind11_init_smtk_bridge_discrete_ArrangementHelper(discrete);
  // PySharedPtrClass< smtk::bridge::discrete::Operation, smtk::operation::Operation > smtk_bridge_discrete_Operation = pybind11_init_smtk_bridge_discrete_Operator(discrete);
  PySharedPtrClass< smtk::bridge::discrete::Session, smtk::model::Session > smtk_bridge_discrete_Session = pybind11_init_smtk_bridge_discrete_Session(discrete);

  py::class_< smtk::bridge::discrete::Registrar > smtk_bridge_discrete_Registrar = pybind11_init_smtk_bridge_discrete_Registrar(discrete);
}
