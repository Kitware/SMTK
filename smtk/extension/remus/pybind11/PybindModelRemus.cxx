//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <utility>
#include <pybind11/pybind11.h>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindMeshOperator.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_PLUGIN(smtkPybindModelRemus)
{
  py::module remus("smtkPybindModelRemus", "<description>");

  py::class_< smtk::model::MeshOperator, smtk::model::Operator > smtk_extension_remus_MeshOperator = pybind11_init_smtk_extension_remus_MeshOperator(remus);

  return remus.ptr();
}
