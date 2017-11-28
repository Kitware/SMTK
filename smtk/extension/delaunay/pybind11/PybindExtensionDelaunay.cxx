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

#include "PybindRegisterOperations.h"
#include "PybindTessellateFaces.h"
#include "PybindTriangulateFaces.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindExtensionDelaunay, delaunay)
{
  delaunay.doc() = "<description>";

  py::class_< smtk::extension::delaunay::TessellateFaces, smtk::operation::XMLOperator > smtk_extension_delaunay_TessellateFaces = pybind11_init_smtk_extension_delaunay_TessellateFaces(delaunay);
  py::class_< smtk::extension::delaunay::TriangulateFaces, smtk::operation::XMLOperator > smtk_extension_delaunay_TriangulateFaces = pybind11_init_smtk_extension_delaunay_TriangulateFaces(delaunay);
  pybind11_init__extension_delaunay_registerOperations(delaunay);
}
