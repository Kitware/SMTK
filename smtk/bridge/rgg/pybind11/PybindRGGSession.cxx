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
#include "PybindSession.h"

PYBIND11_MODULE(_smtkPybindRGGSession, rgg)
{
  rgg.doc() = "<description>";

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::bridge::rgg::Operator, smtk::model::Operator > smtk_bridge_rgg_Operator = pybind11_init_smtk_bridge_rgg_Operator(rgg);
  PySharedPtrClass< smtk::bridge::rgg::Session, smtk::model::Session > smtk_bridge_rgg_Session = pybind11_init_smtk_bridge_rgg_Session(rgg);

}
