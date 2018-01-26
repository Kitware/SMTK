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

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindMultiscaleSession, multiscale)
{
  multiscale.doc() = "<description>";

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::bridge::multiscale::Session, smtk::bridge::mesh::Session > smtk_bridge_multiscale_Session = pybind11_init_smtk_bridge_multiscale_Session(multiscale);

  PySharedPtrClass< smtk::bridge::multiscale::Revolve, smtk::operation::XMLOperator > smtk_bridge_multiscale_Revolve = pybind11_init_smtk_bridge_multiscale_Revolve(multiscale);
  PySharedPtrClass< smtk::bridge::multiscale::PartitionBoundaries, smtk::operation::XMLOperator > smtk_bridge_multiscale_PartitionBoundaries = pybind11_init_smtk_bridge_multiscale_PartitionBoundaries(multiscale);
}
