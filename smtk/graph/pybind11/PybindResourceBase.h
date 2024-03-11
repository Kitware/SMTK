//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_graph_ResourceBase_h
#define pybind_smtk_graph_ResourceBase_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/graph/ResourceBase.h"

#include "smtk/string/Token.h"
#include "smtk/geometry/Resource.h"

#include <vector>
#include <string>

namespace py = pybind11;

inline PySharedPtrClass< smtk::graph::ResourceBase> pybind11_init_smtk_graph_ResourceBase(py::module &m)
{
  PySharedPtrClass< smtk::graph::ResourceBase, smtk::geometry::Resource> instance(m, "ResourceBase");
  instance
    .def("arcs", (smtk::graph::ArcMap&(smtk::graph::ResourceBase::*)())&smtk::graph::ResourceBase::arcs, py::return_value_policy::reference_internal)
    .def("arcTypes", &smtk::graph::ResourceBase::arcTypes)
    .def("nodeTypes", &smtk::graph::ResourceBase::nodeTypes)
    .def("createNodeOfType", &smtk::graph::ResourceBase::createNodeOfType, py::arg("nodeType"))
    .def("connect", &smtk::graph::ResourceBase::connect, py::arg("from"), py::arg("to"), py::arg("arcType"))
    .def("disconnect", &smtk::graph::ResourceBase::disconnect, py::arg("from"), py::arg("explicitOnly") = false)
    .def("dump", &smtk::graph::ResourceBase::dump, py::arg("filename"), py::arg("mimeType") = "text/vnd.graphviz")
    ;
  return instance;
}

#endif
