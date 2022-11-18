//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_graph_Component_h
#define pybind_smtk_graph_Component_h

#include <pybind11/pybind11.h>

#include "smtk/graph/Component.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::graph::Component> pybind11_init_smtk_graph_Component(py::module &m)
{
  PySharedPtrClass< smtk::graph::Component, smtk::resource::Component> instance(m, "Component");
  instance
    .def("setId", &smtk::graph::Component::setId, py::arg("uuid"))
    .def("disconnect", &smtk::graph::Component::disconnect, py::arg("onlyExplicit") = false)
    ;
  return instance;
}

#endif
