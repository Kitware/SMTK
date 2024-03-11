//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_graph_ArcMap_h
#define pybind_smtk_graph_ArcMap_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/graph/ArcMap.h"

#include "smtk/graph/Directionality.h"
#include "smtk/graph/Resource.h"

#include <vector>
#include <string>

namespace py = pybind11;

inline PySharedPtrClass< smtk::graph::ArcMap> pybind11_init_smtk_graph_ArcMap(py::module &m)
{
  PySharedPtrClass< smtk::graph::ArcMap> instance(m, "ArcMap");
  instance
    .def("runtimeBaseTypes", &smtk::graph::ArcMap::runtimeBaseTypes)
    .def("runtimeTypeNames", &smtk::graph::ArcMap::runtimeTypeNames, py::arg("baseType"))
    // Convenience methods to make arc-type insertion easier
    .def("insertDirectedRuntimeArcType", [](
        smtk::graph::ArcMap& arcMap,
        const std::shared_ptr<smtk::graph::ResourceBase>& resource,
        const std::string& arcType,
        const std::vector<std::string>& fromNodeSpecs,
        const std::vector<std::string>& toNodeSpecs
        )
      {
        std::unordered_set<smtk::string::Token> fromNodeTokens;
        std::unordered_set<smtk::string::Token> toNodeTokens;
        for (const auto& spec : fromNodeSpecs) { fromNodeTokens.insert(smtk::string::Token(spec)); }
        for (const auto& spec : toNodeSpecs) { toNodeTokens.insert(smtk::string::Token(spec)); }
        return arcMap.insertRuntimeArcType(
          resource.get(), arcType, fromNodeTokens, toNodeTokens, smtk::graph::Directionality::IsDirected);
      },
      py::arg("resource"),
      py::arg("arcType"),
      py::arg("fromNodeSpecs"),
      py::arg("toNodeSpecs")
    )
    .def("insertUndirectedRuntimeArcType", [](
        smtk::graph::ArcMap& arcMap,
        const std::shared_ptr<smtk::graph::ResourceBase>& resource,
        const std::string& arcType,
        const std::vector<std::string>& nodeSpecs
        )
      {
        std::unordered_set<smtk::string::Token> nodeTokens;
        for (const auto& spec : nodeSpecs) { nodeTokens.insert(smtk::string::Token(spec)); }
        return arcMap.insertRuntimeArcType(
          resource.get(), arcType, nodeTokens, nodeTokens, smtk::graph::Directionality::IsUndirected);
      },
      py::arg("resource"),
      py::arg("arcType"),
      py::arg("nodeSpecs")
    )
    ;
  return instance;
}

#endif
