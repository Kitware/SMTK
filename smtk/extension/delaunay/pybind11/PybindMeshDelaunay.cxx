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

#include "PybindTessellateFace.h"
#include "PybindTriangulateFace.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindMeshDelaunay, delaunay)
{
  delaunay.doc() = "<description>";

  py::class_< smtk::mesh::TessellateFace, smtk::model::Operator > smtk_extension_delaunay_TessellateFace = pybind11_init_smtk_extension_delaunay_TessellateFace(delaunay);
  py::class_< smtk::mesh::TriangulateFace, smtk::model::Operator > smtk_extension_delaunay_TriangulateFace = pybind11_init_smtk_extension_delaunay_TriangulateFace(delaunay);
}
