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

#include "PybindTriangulateFace.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_PLUGIN(_smtkPybindModelDelaunay)
{
  py::module delaunay("_smtkPybindModelDelaunay", "<description>");

  py::class_< smtk::model::TriangulateFace, smtk::model::Operator > smtk_extension_delaunay_TriangulateFace = pybind11_init_smtk_extension_delaunay_TriangulateFace(delaunay);

  return delaunay.ptr();
}
