//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_Collection_h
#define pybind_smtk_mesh_Collection_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/Collection.h"

#include "smtk/mesh/Manager.h"
#include "smtk/model/EntityIterator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::Collection > pybind11_init_smtk_mesh_Collection(py::module &m)
{
  PySharedPtrClass< smtk::mesh::Collection > instance(m, "Collection");
  instance
    .def_property("modelManager", &smtk::mesh::Collection::modelManager, &smtk::mesh::Collection::setModelManager)
    .def("assignUniqueNameIfNotAlready", &smtk::mesh::Collection::assignUniqueNameIfNotAlready)
    .def("associateToModel", &smtk::mesh::Collection::associateToModel, py::arg("uuid"))
    .def("associatedModel", &smtk::mesh::Collection::associatedModel)
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::Collection::*)() const) &smtk::mesh::Collection::cells)
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::Collection::*)(::smtk::mesh::CellType) const) &smtk::mesh::Collection::cells, py::arg("cellType"))
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::Collection::*)(::smtk::mesh::CellTypes) const) &smtk::mesh::Collection::cells, py::arg("cellTypes"))
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::Collection::*)(::smtk::mesh::DimensionType) const) &smtk::mesh::Collection::cells, py::arg("dim"))
    .def("classname", &smtk::mesh::Collection::classname)
    .def("clearReadWriteLocations", &smtk::mesh::Collection::clearReadWriteLocations)
    .def_static("create", (std::shared_ptr<smtk::mesh::Collection> (*)()) &smtk::mesh::Collection::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::Collection> (*)(::std::shared_ptr<smtk::mesh::Collection> &)) &smtk::mesh::Collection::create, py::arg("ref"))
    .def("createMesh", &smtk::mesh::Collection::createMesh, py::arg("cells"))
    .def("dirichletMeshes", &smtk::mesh::Collection::dirichletMeshes, py::arg("d"))
    .def("dirichlets", &smtk::mesh::Collection::dirichlets)
    .def("domainMeshes", &smtk::mesh::Collection::domainMeshes, py::arg("m"))
    .def("domains", &smtk::mesh::Collection::domains)
    .def("entity", &smtk::mesh::Collection::entity)
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Collection::*)(::smtk::model::EntityRef const &) const) &smtk::mesh::Collection::findAssociatedCells, py::arg("eref"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Collection::*)(::smtk::model::EntityRef const &, ::smtk::mesh::CellType) const) &smtk::mesh::Collection::findAssociatedCells, py::arg("eref"), py::arg("cellType"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Collection::*)(::smtk::model::EntityRef const &, ::smtk::mesh::DimensionType) const) &smtk::mesh::Collection::findAssociatedCells, py::arg("eref"), py::arg("dim"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Collection::*)(::smtk::common::UUID const &) const) &smtk::mesh::Collection::findAssociatedCells, py::arg("id"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Collection::*)(::smtk::common::UUID const &, ::smtk::mesh::CellType) const) &smtk::mesh::Collection::findAssociatedCells, py::arg("id"), py::arg("cellType"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Collection::*)(::smtk::common::UUID const &, ::smtk::mesh::DimensionType) const) &smtk::mesh::Collection::findAssociatedCells, py::arg("id"), py::arg("dim"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Collection::*)(::smtk::model::EntityIterator &) const) &smtk::mesh::Collection::findAssociatedCells, py::arg("refIt"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Collection::*)(::smtk::model::EntityIterator &, ::smtk::mesh::CellType) const) &smtk::mesh::Collection::findAssociatedCells, py::arg("refIt"), py::arg("cellType"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Collection::*)(::smtk::model::EntityIterator &, ::smtk::mesh::DimensionType) const) &smtk::mesh::Collection::findAssociatedCells, py::arg("refIt"), py::arg("dim"))
    .def("findAssociatedMeshes", (smtk::mesh::MeshSet (smtk::mesh::Collection::*)(::smtk::model::EntityRef const &) const) &smtk::mesh::Collection::findAssociatedMeshes, py::arg("eref"))
    .def("findAssociatedMeshes", (smtk::mesh::MeshSet (smtk::mesh::Collection::*)(::smtk::model::EntityRef const &, ::smtk::mesh::DimensionType) const) &smtk::mesh::Collection::findAssociatedMeshes, py::arg("eref"), py::arg("dim"))
    .def("findAssociatedMeshes", (smtk::mesh::MeshSet (smtk::mesh::Collection::*)(::smtk::common::UUID const &) const) &smtk::mesh::Collection::findAssociatedMeshes, py::arg("id"))
    .def("findAssociatedMeshes", (smtk::mesh::MeshSet (smtk::mesh::Collection::*)(::smtk::common::UUID const &, ::smtk::mesh::DimensionType) const) &smtk::mesh::Collection::findAssociatedMeshes, py::arg("id"), py::arg("dim"))
    .def("findAssociatedMeshes", (smtk::mesh::MeshSet (smtk::mesh::Collection::*)(::smtk::model::EntityIterator &) const) &smtk::mesh::Collection::findAssociatedMeshes, py::arg("refIt"))
    .def("findAssociatedMeshes", (smtk::mesh::MeshSet (smtk::mesh::Collection::*)(::smtk::model::EntityIterator &, ::smtk::mesh::DimensionType) const) &smtk::mesh::Collection::findAssociatedMeshes, py::arg("refIt"), py::arg("dim"))
    .def("findAssociatedTypes", (smtk::mesh::TypeSet (smtk::mesh::Collection::*)(::smtk::model::EntityRef const &) const) &smtk::mesh::Collection::findAssociatedTypes, py::arg("eref"))
    .def("findAssociatedTypes", (smtk::mesh::TypeSet (smtk::mesh::Collection::*)(::smtk::common::UUID const &) const) &smtk::mesh::Collection::findAssociatedTypes, py::arg("id"))
    .def("findAssociatedTypes", (smtk::mesh::TypeSet (smtk::mesh::Collection::*)(::smtk::model::EntityIterator &) const) &smtk::mesh::Collection::findAssociatedTypes, py::arg("refIt"))
    .def("floatProperty", (smtk::model::FloatList const & (smtk::mesh::Collection::*)(::smtk::mesh::MeshSet const &, ::std::string const &) const) &smtk::mesh::Collection::floatProperty, py::arg("meshset"), py::arg("propName"))
    .def("floatProperty", (smtk::model::FloatList & (smtk::mesh::Collection::*)(::smtk::mesh::MeshSet const &, ::std::string const &)) &smtk::mesh::Collection::floatProperty, py::arg("meshset"), py::arg("propName"))
    .def("hasAssociations", &smtk::mesh::Collection::hasAssociations)
    .def("hasFloatProperty", &smtk::mesh::Collection::hasFloatProperty, py::arg("meshset"), py::arg("propName"))
    .def("hasIntegerProperty", &smtk::mesh::Collection::hasIntegerProperty, py::arg("meshset"), py::arg("propName"))
    .def("hasStringProperty", &smtk::mesh::Collection::hasStringProperty, py::arg("meshset"), py::arg("propName"))
    .def("integerProperty", (smtk::model::IntegerList const & (smtk::mesh::Collection::*)(::smtk::mesh::MeshSet const &, ::std::string const &) const) &smtk::mesh::Collection::integerProperty, py::arg("meshset"), py::arg("propName"))
    .def("integerProperty", (smtk::model::IntegerList & (smtk::mesh::Collection::*)(::smtk::mesh::MeshSet const &, ::std::string const &)) &smtk::mesh::Collection::integerProperty, py::arg("meshset"), py::arg("propName"))
    .def("interface", &smtk::mesh::Collection::interface)
    .def("interfaceName", &smtk::mesh::Collection::interfaceName)
    .def("isAssociatedToModel", &smtk::mesh::Collection::isAssociatedToModel)
    .def("isModified", &smtk::mesh::Collection::isModified)
    .def("isValid", &smtk::mesh::Collection::isValid)
    .def("meshNames", &smtk::mesh::Collection::meshNames)
    .def("meshes", (smtk::mesh::MeshSet (smtk::mesh::Collection::*)() const) &smtk::mesh::Collection::meshes)
    .def("meshes", (smtk::mesh::MeshSet (smtk::mesh::Collection::*)(::smtk::mesh::DimensionType) const) &smtk::mesh::Collection::meshes, py::arg("dim"))
    .def("meshes", (smtk::mesh::MeshSet (smtk::mesh::Collection::*)(::smtk::mesh::Domain const &) const) &smtk::mesh::Collection::meshes, py::arg("d"))
    .def("meshes", (smtk::mesh::MeshSet (smtk::mesh::Collection::*)(::smtk::mesh::Dirichlet const &) const) &smtk::mesh::Collection::meshes, py::arg("d"))
    .def("meshes", (smtk::mesh::MeshSet (smtk::mesh::Collection::*)(::smtk::mesh::Neumann const &) const) &smtk::mesh::Collection::meshes, py::arg("n"))
    .def("meshes", (smtk::mesh::MeshSet (smtk::mesh::Collection::*)(::std::string const &) const) &smtk::mesh::Collection::meshes, py::arg("name"))
    .def("name", (std::string const & (smtk::mesh::Collection::*)() const) &smtk::mesh::Collection::name)
    .def("name", (void (smtk::mesh::Collection::*)(::std::string const &)) &smtk::mesh::Collection::name, py::arg("n"))
    .def("neumannMeshes", &smtk::mesh::Collection::neumannMeshes, py::arg("n"))
    .def("neumanns", &smtk::mesh::Collection::neumanns)
    .def("numberOfMeshes", &smtk::mesh::Collection::numberOfMeshes)
    .def("pointConnectivity", &smtk::mesh::Collection::pointConnectivity)
    .def("points", &smtk::mesh::Collection::points)
    .def("readLocation", (const smtk::common::FileLocation& (smtk::mesh::Collection::*)() const) &smtk::mesh::Collection::readLocation)
    .def("removeFloatProperty", &smtk::mesh::Collection::removeFloatProperty, py::arg("meshset"), py::arg("propName"))
    .def("removeIntegerProperty", &smtk::mesh::Collection::removeIntegerProperty, py::arg("meshset"), py::arg("propName"))
    .def("removeMeshes", &smtk::mesh::Collection::removeMeshes, py::arg("meshesToDelete"))
    .def("removeStringProperty", &smtk::mesh::Collection::removeStringProperty, py::arg("meshset"), py::arg("propName"))
    .def("reparent", &smtk::mesh::Collection::reparent, py::arg("newParent"))
    .def("setAssociation", &smtk::mesh::Collection::setAssociation, py::arg("eref"), py::arg("meshset"))
    .def("setDirichletOnMeshes", &smtk::mesh::Collection::setDirichletOnMeshes, py::arg("meshes"), py::arg("d"))
    .def("setDomainOnMeshes", &smtk::mesh::Collection::setDomainOnMeshes, py::arg("meshes"), py::arg("m"))
    .def("setFloatProperty", (void (smtk::mesh::Collection::*)(::smtk::mesh::MeshSet const &, ::std::string const &, ::smtk::model::Float)) &smtk::mesh::Collection::setFloatProperty, py::arg("meshset"), py::arg("propName"), py::arg("propValue"))
    .def("setFloatProperty", (void (smtk::mesh::Collection::*)(::smtk::mesh::MeshSet const &, ::std::string const &, ::smtk::model::FloatList const &)) &smtk::mesh::Collection::setFloatProperty, py::arg("meshset"), py::arg("propName"), py::arg("propValue"))
    .def("setIntegerProperty", (void (smtk::mesh::Collection::*)(::smtk::mesh::MeshSet const &, ::std::string const &, ::smtk::model::Integer)) &smtk::mesh::Collection::setIntegerProperty, py::arg("meshset"), py::arg("propName"), py::arg("propValue"))
    .def("setIntegerProperty", (void (smtk::mesh::Collection::*)(::smtk::mesh::MeshSet const &, ::std::string const &, ::smtk::model::IntegerList const &)) &smtk::mesh::Collection::setIntegerProperty, py::arg("meshset"), py::arg("propName"), py::arg("propValue"))
    .def("setNeumannOnMeshes", &smtk::mesh::Collection::setNeumannOnMeshes, py::arg("meshes"), py::arg("n"))
    .def("setStringProperty", (void (smtk::mesh::Collection::*)(::smtk::mesh::MeshSet const &, ::std::string const &, ::smtk::model::String const &)) &smtk::mesh::Collection::setStringProperty, py::arg("meshset"), py::arg("propName"), py::arg("propValue"))
    .def("setStringProperty", (void (smtk::mesh::Collection::*)(::smtk::mesh::MeshSet const &, ::std::string const &, ::smtk::model::StringList const &)) &smtk::mesh::Collection::setStringProperty, py::arg("meshset"), py::arg("propName"), py::arg("propValue"))
    .def("stringProperty", (smtk::model::StringList const & (smtk::mesh::Collection::*)(::smtk::mesh::MeshSet const &, ::std::string const &) const) &smtk::mesh::Collection::stringProperty, py::arg("meshset"), py::arg("propName"))
    .def("stringProperty", (smtk::model::StringList & (smtk::mesh::Collection::*)(::smtk::mesh::MeshSet const &, ::std::string const &)) &smtk::mesh::Collection::stringProperty, py::arg("meshset"), py::arg("propName"))
    .def("types", &smtk::mesh::Collection::types)
    .def("writeLocation", (void (smtk::mesh::Collection::*)(::smtk::common::FileLocation const &)) &smtk::mesh::Collection::writeLocation, py::arg("path"))
    .def("writeLocation", (void (smtk::mesh::Collection::*)(::std::string const &)) &smtk::mesh::Collection::writeLocation, py::arg("path"))
    .def("writeLocation", (smtk::common::FileLocation const & (smtk::mesh::Collection::*)() const) &smtk::mesh::Collection::writeLocation)
    ;
  return instance;
}

#endif
