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

#include "PybindArcMap.h"
#include "PybindResourceBase.h"
#include "PybindComponent.h"

#include "smtk/resource/Manager.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindGraph, graph)
{
  graph.doc() = "<description>";

  py::module::import("smtk.common");
  py::module::import("smtk.string");
  py::module::import("smtk.resource");
  py::module::import("smtk.geometry");

  auto smtk_graph_ResourceBase = pybind11_init_smtk_graph_ResourceBase(graph);
  auto smtk_graph_Component = pybind11_init_smtk_graph_Component(graph);
  auto smtk_graph_ArcMap = pybind11_init_smtk_graph_ArcMap(graph);
}
