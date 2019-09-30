//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_mesh_Topology_h
#define pybind_smtk_session_mesh_Topology_h

#include <pybind11/pybind11.h>

#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/session/mesh/Topology.h"

namespace py = pybind11;

py::class_< smtk::session::mesh::Topology > pybind11_init_smtk_session_mesh_Topology(py::module &m)
{
  py::class_< smtk::session::mesh::Topology > instance(m, "Topology");
  instance
    .def(py::init<::smtk::session::mesh::Topology const &>())
    .def(py::init<::smtk::common::UUID const, ::smtk::mesh::MeshSet const &, bool>())
    .def("deepcopy", (smtk::session::mesh::Topology & (smtk::session::mesh::Topology::*)(::smtk::session::mesh::Topology const &)) &smtk::session::mesh::Topology::operator=)
    .def("resource", [](const smtk::session::mesh::Topology& topology){ return topology.m_resource; })
    ;
  py::class_< smtk::session::mesh::Topology::Element >(instance, "Element")
    .def(py::init<::smtk::mesh::MeshSet, smtk::common::UUID const &, int>())
    .def(py::init<::smtk::session::mesh::Topology::Element const &>())
    .def("deepcopy", (smtk::session::mesh::Topology::Element & (smtk::session::mesh::Topology::Element::*)(::smtk::session::mesh::Topology::Element const &)) &smtk::session::mesh::Topology::Element::operator=)
    .def_readwrite("m_mesh", &smtk::session::mesh::Topology::Element::m_mesh)
    .def_readwrite("m_dimension", &smtk::session::mesh::Topology::Element::m_dimension)
    .def_readwrite("m_parents", &smtk::session::mesh::Topology::Element::m_parents)
    .def_readwrite("m_children", &smtk::session::mesh::Topology::Element::m_children)
    ;
  return instance;
}

#endif
