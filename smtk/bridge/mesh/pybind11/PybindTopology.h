//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_mesh_Topology_h
#define pybind_smtk_bridge_mesh_Topology_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/mesh/Topology.h"

namespace py = pybind11;

py::class_< smtk::bridge::mesh::Topology > pybind11_init_smtk_bridge_mesh_Topology(py::module &m)
{
  py::class_< smtk::bridge::mesh::Topology > instance(m, "Topology");
  instance
    .def(py::init<::smtk::mesh::CollectionPtr>())
    .def(py::init<::smtk::bridge::mesh::Topology const &>())
    .def("deepcopy", (smtk::bridge::mesh::Topology & (smtk::bridge::mesh::Topology::*)(::smtk::bridge::mesh::Topology const &)) &smtk::bridge::mesh::Topology::operator=)
    .def_readwrite("m_collection", &smtk::bridge::mesh::Topology::m_collection)
    .def_readwrite("m_elements", &smtk::bridge::mesh::Topology::m_elements)
    ;
  py::class_< smtk::bridge::mesh::Topology::Element >(instance, "Element")
    .def(py::init<int>())
    .def(py::init<::smtk::bridge::mesh::Topology::Element const &>())
    .def("deepcopy", (smtk::bridge::mesh::Topology::Element & (smtk::bridge::mesh::Topology::Element::*)(::smtk::bridge::mesh::Topology::Element const &)) &smtk::bridge::mesh::Topology::Element::operator=)
    .def_readwrite("m_dimension", &smtk::bridge::mesh::Topology::Element::m_dimension)
    .def_readwrite("m_children", &smtk::bridge::mesh::Topology::Element::m_children)
    ;
  return instance;
}

#endif
