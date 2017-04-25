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

#include "PybindOperator.h"
#include "PybindPointerDefs.h"
#include "PybindSession.h"

#include "PybindDream3DPipeline.h"
#include "PybindPartitionBoundaries.h"
#include "PybindPythonScript.h"
#include "PybindRevolve.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_PLUGIN(smtkPybindMultiscaleSession)
{
  py::module multiscale("smtkPybindMultiscaleSession", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::bridge::multiscale::Operator, smtk::model::Operator > smtk_bridge_multiscale_Operator = pybind11_init_smtk_bridge_multiscale_Operator(multiscale);
  PySharedPtrClass< smtk::bridge::multiscale::Session, smtk::bridge::mesh::Session > smtk_bridge_multiscale_Session = pybind11_init_smtk_bridge_multiscale_Session(multiscale);

  PySharedPtrClass< smtk::bridge::multiscale::PythonScript, smtk::bridge::multiscale::Operator > smtk_bridge_multiscale_PythonScript = pybind11_init_smtk_bridge_multiscale_PythonScript(multiscale);
  PySharedPtrClass< smtk::bridge::multiscale::Revolve, smtk::bridge::multiscale::Operator > smtk_bridge_multiscale_Revolve = pybind11_init_smtk_bridge_multiscale_Revolve(multiscale);
  PySharedPtrClass< smtk::bridge::multiscale::Dream3DPipeline, smtk::bridge::multiscale::PythonScript > smtk_bridge_multiscale_Dream3DPipeline = pybind11_init_smtk_bridge_multiscale_Dream3DPipeline(multiscale);
  PySharedPtrClass< smtk::bridge::multiscale::PartitionBoundaries, smtk::bridge::multiscale::Operator > smtk_bridge_multiscale_PartitionBoundaries = pybind11_init_smtk_bridge_multiscale_PartitionBoundaries(multiscale);

  return multiscale.ptr();
}
