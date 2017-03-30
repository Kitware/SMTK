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
#include "PybindOperator.h"
#include "PybindSession.h"
#include "PybindSession_json.h"

#include "PybindCreateEdgesOperator.h"
#include "PybindEdgeOperator.h"
#include "PybindEntityGroupOperator.h"
#include "PybindGrowOperator.h"
#include "PybindImportOperator.h"
#include "PybindMergeOperator.h"
#include "PybindReadOperator.h"
#include "PybindRemoveModel.h"
#include "PybindSetProperty.h"
#include "PybindSplitFaceOperator.h"
#include "PybindWriteOperator.h"

#include "smtk/mesh/ForEachTypes.h"

#include "smtk/model/ArrangementHelper.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_PLUGIN(_smtkPybindDiscreteSession)
{
  py::module discrete("_smtkPybindDiscreteSession", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::bridge::discrete::ArrangementHelper, smtk::model::ArrangementHelper > smtk_bridge_discrete_ArrangementHelper = pybind11_init_smtk_bridge_discrete_ArrangementHelper(discrete);
  PySharedPtrClass< smtk::bridge::discrete::Operator, smtk::model::Operator > smtk_bridge_discrete_Operator = pybind11_init_smtk_bridge_discrete_Operator(discrete);
  PySharedPtrClass< smtk::bridge::discrete::Session, smtk::model::Session > smtk_bridge_discrete_Session = pybind11_init_smtk_bridge_discrete_Session(discrete);

  PySharedPtrClass< smtk::bridge::discrete::CreateEdgesOperator, smtk::model::Operator > smtk_bridge_discrete_CreateEdgesOperator = pybind11_init_smtk_bridge_discrete_CreateEdgesOperator(discrete);
  PySharedPtrClass< smtk::bridge::discrete::EdgeOperator, smtk::model::Operator > smtk_bridge_discrete_EdgeOperator = pybind11_init_smtk_bridge_discrete_EdgeOperator(discrete);
  PySharedPtrClass< smtk::bridge::discrete::EntityGroupOperator, smtk::model::Operator > smtk_bridge_discrete_EntityGroupOperator = pybind11_init_smtk_bridge_discrete_EntityGroupOperator(discrete);
  PySharedPtrClass< smtk::bridge::discrete::GrowOperator, smtk::model::Operator > smtk_bridge_discrete_GrowOperator = pybind11_init_smtk_bridge_discrete_GrowOperator(discrete);
  PySharedPtrClass< smtk::bridge::discrete::ImportOperator, smtk::model::Operator > smtk_bridge_discrete_ImportOperator = pybind11_init_smtk_bridge_discrete_ImportOperator(discrete);
  PySharedPtrClass< smtk::bridge::discrete::MergeOperator, smtk::model::Operator > smtk_bridge_discrete_MergeOperator = pybind11_init_smtk_bridge_discrete_MergeOperator(discrete);
  PySharedPtrClass< smtk::bridge::discrete::ReadOperator, smtk::model::Operator > smtk_bridge_discrete_ReadOperator = pybind11_init_smtk_bridge_discrete_ReadOperator(discrete);
  PySharedPtrClass< smtk::bridge::discrete::RemoveModel, smtk::model::Operator > smtk_bridge_discrete_RemoveModel = pybind11_init_smtk_bridge_discrete_RemoveModel(discrete);
  PySharedPtrClass< smtk::bridge::discrete::SetProperty, smtk::model::Operator > smtk_bridge_discrete_SetProperty = pybind11_init_smtk_bridge_discrete_SetProperty(discrete);
  PySharedPtrClass< smtk::bridge::discrete::SplitFaceOperator, smtk::model::Operator > smtk_bridge_discrete_SplitFaceOperator = pybind11_init_smtk_bridge_discrete_SplitFaceOperator(discrete);
  PySharedPtrClass< smtk::bridge::discrete::WriteOperator, smtk::model::Operator > smtk_bridge_discrete_WriteOperator = pybind11_init_smtk_bridge_discrete_WriteOperator(discrete);

  return discrete.ptr();
}
