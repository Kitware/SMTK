//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_Manager_h
#define pybind_smtk_mesh_Manager_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/Manager.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::Manager > pybind11_init_smtk_mesh_Manager(py::module &m)
{
  PySharedPtrClass< smtk::mesh::Manager > instance(m, "Manager");
  instance
    .def(py::init<::smtk::mesh::Manager const &>())
    .def("deepcopy", (smtk::mesh::Manager & (smtk::mesh::Manager::*)(::smtk::mesh::Manager const &)) &smtk::mesh::Manager::operator=)
    .def("classname", &smtk::mesh::Manager::classname)
    .def_static("create", (std::shared_ptr<smtk::mesh::Manager> (*)()) &smtk::mesh::Manager::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::Manager> (*)(::std::shared_ptr<smtk::mesh::Manager> &)) &smtk::mesh::Manager::create, py::arg("ref"))
    .def("makeCollection", (smtk::mesh::CollectionPtr (smtk::mesh::Manager::*)()) &smtk::mesh::Manager::makeCollection)
    .def("makeCollection", (smtk::mesh::CollectionPtr (smtk::mesh::Manager::*)(::smtk::common::UUID const &)) &smtk::mesh::Manager::makeCollection, py::arg("collectionID"))
    .def("makeCollection", (smtk::mesh::CollectionPtr (smtk::mesh::Manager::*)(::smtk::mesh::InterfacePtr)) &smtk::mesh::Manager::makeCollection, py::arg("interface"))
    .def("makeCollection", (smtk::mesh::CollectionPtr (smtk::mesh::Manager::*)(::smtk::common::UUID const &, ::smtk::mesh::InterfacePtr)) &smtk::mesh::Manager::makeCollection, py::arg("collectionID"), py::arg("interface"))
    .def("numberOfCollections", &smtk::mesh::Manager::numberOfCollections)
    .def("hasCollection", &smtk::mesh::Manager::hasCollection, py::arg("collection"))
    .def("collectionBegin", &smtk::mesh::Manager::collectionBegin)
    .def("collectionEnd", &smtk::mesh::Manager::collectionEnd)
    .def("findCollection", &smtk::mesh::Manager::findCollection, py::arg("collectionID"))
    .def("collection", &smtk::mesh::Manager::collection, py::arg("collectionID"))
    .def("removeCollection", &smtk::mesh::Manager::removeCollection, py::arg("collection"))
    .def("collectionsWithAssociations", &smtk::mesh::Manager::collectionsWithAssociations)
    .def("isAssociatedToACollection", &smtk::mesh::Manager::isAssociatedToACollection, py::arg("eref"))
    .def("associatedCollections", &smtk::mesh::Manager::associatedCollections, py::arg("c"))
    .def("associatedCollectionIds", &smtk::mesh::Manager::associatedCollectionIds, py::arg("c"))
    .def("assignUniqueName", &smtk::mesh::Manager::assignUniqueName, py::arg("collection"))
    ;
  return instance;
}

#endif
