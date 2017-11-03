//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_Reclassify_h
#define pybind_smtk_mesh_Reclassify_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/utility/Reclassify.h"

namespace py = pybind11;

void pybind11_init_smtk_mesh_split(py::module &m)
{
  m.def("split", &smtk::mesh::utility::split, "", py::arg("arg0"), py::arg("orignalEdge"), py::arg("newEdge"), py::arg("promotedVertex"));
}

void pybind11_init_smtk_mesh_merge(py::module &m)
{
  m.def("merge", &smtk::mesh::utility::merge, "", py::arg("arg0"), py::arg("toRemoveVert"), py::arg("toRemoveEdge"), py::arg("toAddTo"));
}

void pybind11_init_smtk_mesh_make_disjoint(py::module &m)
{
  m.def("make_disjoint", &smtk::mesh::utility::make_disjoint, "", py::arg("arg0"), py::arg("a"), py::arg("b"), py::arg("modelAssoc"));
}

void pybind11_init_smtk_mesh_fuse(py::module &m)
{
  m.def("fuse", &smtk::mesh::utility::fuse, "", py::arg("arg0"), py::arg("toRemove"), py::arg("toAddTo"), py::arg("assoc"));
}

#endif
