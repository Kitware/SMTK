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

#include "PybindResource.h"
#include "PybindComponent.h"
#include "PybindSpatialData.h"
#include "PybindDomain.h"
#include "PybindDomainMap.h"
#include "PybindUnstructuredData.h"

#include "smtk/resource/Manager.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindMarkup, markup)
{
  markup.doc() = "<description>";

  py::module::import("smtk.common");
  py::module::import("smtk.string");
  py::module::import("smtk.resource");
  py::module::import("smtk.geometry");
  py::module::import("smtk.graph");

  auto smtk_markup_Resource = pybind11_init_smtk_markup_Resource(markup);
  auto smtk_markup_Component = pybind11_init_smtk_markup_Component(markup);
  auto smtk_markup_Domain = pybind11_init_smtk_markup_Domain(markup);
  auto smtk_markup_DomainMap = pybind11_init_smtk_markup_DomainMap(markup);
  auto smtk_markup_SpatialData = pybind11_init_smtk_markup_SpatialData(markup);
  auto smtk_markup_UnstructuredData = pybind11_init_smtk_markup_UnstructuredData(markup);
}
