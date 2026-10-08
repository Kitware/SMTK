//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_graph_RuntimeArcEndpoint_h
#define pybind_smtk_graph_RuntimeArcEndpoint_h

#include <pybind11/pybind11.h>

#include "smtk/graph/RuntimeArcEndpoint.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

inline py::class_<smtk::graph::RuntimeArcEndpoint<smtk::graph::NonConstArc>>
pybind11_init_smtk_graph_RuntimeArcEndpoint(py::module &m)
{
  using RuntimeArcEndpoint = smtk::graph::RuntimeArcEndpoint<smtk::graph::NonConstArc>;
  py::class_<RuntimeArcEndpoint> instance(m, "RuntimeArcEndpoint");
  instance
    .def("valid", &RuntimeArcEndpoint::valid)
    .def("self", &RuntimeArcEndpoint::self)
    .def("max_degree", &RuntimeArcEndpoint::maxDegree)
    .def("min_degree", &RuntimeArcEndpoint::minDegree)
    .def("contains", [](const RuntimeArcEndpoint& self, const smtk::graph::Component* node)
      { return self.contains(node); }, py::arg("node"))
    .def("degree", &RuntimeArcEndpoint::degree)
    .def("size", &RuntimeArcEndpoint::size)
    .def("empty", &RuntimeArcEndpoint::empty)
    .def("connect", [](RuntimeArcEndpoint& self, const smtk::graph::Component* node)
      { return self.connect(node); }, py::arg("node"))
    .def("disconnect", [](RuntimeArcEndpoint& self, const smtk::graph::Component* node)
      { return self.disconnect(node); }, py::arg("node"))
    ;
  return instance;
}

#endif
