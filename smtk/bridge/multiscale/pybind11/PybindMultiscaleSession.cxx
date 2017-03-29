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

#include "PybindDream3DPipelineOperator.h"
#include "PybindPartitionBoundariesOperator.h"
#include "PybindPythonScriptOperator.h"
#include "PybindRevolveOperator.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_PLUGIN(smtkPybindMultiscaleSession)
{
  py::module multiscale("smtkPybindMultiscaleSession", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::bridge::multiscale::Operator, smtk::model::Operator > smtk_bridge_multiscale_Operator = pybind11_init_smtk_bridge_multiscale_Operator(multiscale);
  PySharedPtrClass< smtk::bridge::multiscale::Session, smtk::bridge::mesh::Session > smtk_bridge_multiscale_Session = pybind11_init_smtk_bridge_multiscale_Session(multiscale);

  PySharedPtrClass< smtk::bridge::multiscale::PythonScriptOperator, smtk::bridge::multiscale::Operator > smtk_bridge_multiscale_PythonScriptOperator = pybind11_init_smtk_bridge_multiscale_PythonScriptOperator(multiscale);
  PySharedPtrClass< smtk::bridge::multiscale::RevolveOperator, smtk::bridge::multiscale::Operator > smtk_bridge_multiscale_RevolveOperator = pybind11_init_smtk_bridge_multiscale_RevolveOperator(multiscale);
  PySharedPtrClass< smtk::bridge::multiscale::Dream3DPipelineOperator, smtk::bridge::multiscale::PythonScriptOperator > smtk_bridge_multiscale_Dream3DPipelineOperator = pybind11_init_smtk_bridge_multiscale_Dream3DPipelineOperator(multiscale);
  PySharedPtrClass< smtk::bridge::multiscale::PartitionBoundariesOperator, smtk::bridge::multiscale::Operator > smtk_bridge_multiscale_PartitionBoundariesOperator = pybind11_init_smtk_bridge_multiscale_PartitionBoundariesOperator(multiscale);

  return multiscale.ptr();
}
