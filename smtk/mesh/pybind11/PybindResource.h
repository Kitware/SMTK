//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_Resource_h
#define pybind_smtk_mesh_Resource_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/core/Resource.h"

#include "smtk/common/UUID.h"

#include "smtk/model/EntityIterator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::Resource > pybind11_init_smtk_mesh_Resource(py::module &m)
{
  PySharedPtrClass< smtk::mesh::Resource > instance(m, "Resource");
  instance
    .def_property("modelResource", &smtk::mesh::Resource::modelResource, &smtk::mesh::Resource::setModelResource)
    .def("associateToModel", &smtk::mesh::Resource::associateToModel, py::arg("uuid"))
    .def("associatedModel", &smtk::mesh::Resource::associatedModel)
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)() const) &smtk::mesh::Resource::cells)
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::mesh::CellType) const) &smtk::mesh::Resource::cells, py::arg("cellType"))
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::mesh::CellTypes) const) &smtk::mesh::Resource::cells, py::arg("cellTypes"))
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::mesh::DimensionType) const) &smtk::mesh::Resource::cells, py::arg("dim"))
    .def("clearReadWriteLocations", &smtk::mesh::Resource::clearReadWriteLocations)
    .def_static("create", (std::shared_ptr<smtk::mesh::Resource> (*)()) &smtk::mesh::Resource::create)
    .def_static("create", (std::shared_ptr<smtk::mesh::Resource> (*)(::std::shared_ptr<smtk::mesh::Resource> &)) &smtk::mesh::Resource::create, py::arg("ref"))
    .def("createMesh", &smtk::mesh::Resource::createMesh, py::arg("cells"), py::arg("uuid") = smtk::common::UUID::null())
    .def("dirichletMeshes", &smtk::mesh::Resource::dirichletMeshes, py::arg("d"))
    .def("dirichlets", &smtk::mesh::Resource::dirichlets)
    .def("domainMeshes", &smtk::mesh::Resource::domainMeshes, py::arg("m"))
    .def("domains", &smtk::mesh::Resource::domains)
    .def("entity", &smtk::mesh::Resource::entity)
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::model::EntityRef const &) const) &smtk::mesh::Resource::findAssociatedCells, py::arg("eref"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::model::EntityRef const &, ::smtk::mesh::CellType) const) &smtk::mesh::Resource::findAssociatedCells, py::arg("eref"), py::arg("cellType"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::model::EntityRef const &, ::smtk::mesh::DimensionType) const) &smtk::mesh::Resource::findAssociatedCells, py::arg("eref"), py::arg("dim"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::common::UUID const &) const) &smtk::mesh::Resource::findAssociatedCells, py::arg("id"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::common::UUID const &, ::smtk::mesh::CellType) const) &smtk::mesh::Resource::findAssociatedCells, py::arg("id"), py::arg("cellType"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::common::UUID const &, ::smtk::mesh::DimensionType) const) &smtk::mesh::Resource::findAssociatedCells, py::arg("id"), py::arg("dim"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::model::EntityIterator &) const) &smtk::mesh::Resource::findAssociatedCells, py::arg("refIt"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::model::EntityIterator &, ::smtk::mesh::CellType) const) &smtk::mesh::Resource::findAssociatedCells, py::arg("refIt"), py::arg("cellType"))
    .def("findAssociatedCells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::model::EntityIterator &, ::smtk::mesh::DimensionType) const) &smtk::mesh::Resource::findAssociatedCells, py::arg("refIt"), py::arg("dim"))
    .def("findAssociatedMeshes", (smtk::mesh::MeshSet (smtk::mesh::Resource::*)(::smtk::model::EntityRef const &) const) &smtk::mesh::Resource::findAssociatedMeshes, py::arg("eref"))
    .def("findAssociatedMeshes", (smtk::mesh::MeshSet (smtk::mesh::Resource::*)(::smtk::model::EntityRef const &, ::smtk::mesh::DimensionType) const) &smtk::mesh::Resource::findAssociatedMeshes, py::arg("eref"), py::arg("dim"))
    .def("findAssociatedMeshes", (smtk::mesh::MeshSet (smtk::mesh::Resource::*)(::smtk::common::UUID const &) const) &smtk::mesh::Resource::findAssociatedMeshes, py::arg("id"))
    .def("findAssociatedMeshes", (smtk::mesh::MeshSet (smtk::mesh::Resource::*)(::smtk::common::UUID const &, ::smtk::mesh::DimensionType) const) &smtk::mesh::Resource::findAssociatedMeshes, py::arg("id"), py::arg("dim"))
    .def("findAssociatedMeshes", (smtk::mesh::MeshSet (smtk::mesh::Resource::*)(::smtk::model::EntityIterator &) const) &smtk::mesh::Resource::findAssociatedMeshes, py::arg("refIt"))
    .def("findAssociatedMeshes", (smtk::mesh::MeshSet (smtk::mesh::Resource::*)(::smtk::model::EntityIterator &, ::smtk::mesh::DimensionType) const) &smtk::mesh::Resource::findAssociatedMeshes, py::arg("refIt"), py::arg("dim"))
    .def("findAssociatedTypes", (smtk::mesh::TypeSet (smtk::mesh::Resource::*)(::smtk::model::EntityRef const &) const) &smtk::mesh::Resource::findAssociatedTypes, py::arg("eref"))
    .def("findAssociatedTypes", (smtk::mesh::TypeSet (smtk::mesh::Resource::*)(::smtk::common::UUID const &) const) &smtk::mesh::Resource::findAssociatedTypes, py::arg("id"))
    .def("findAssociatedTypes", (smtk::mesh::TypeSet (smtk::mesh::Resource::*)(::smtk::model::EntityIterator &) const) &smtk::mesh::Resource::findAssociatedTypes, py::arg("refIt"))
    .def("floatProperty", (smtk::model::FloatList const & (smtk::mesh::Resource::*)(::smtk::mesh::MeshSet const &, ::std::string const &) const) &smtk::mesh::Resource::floatProperty, py::arg("meshset"), py::arg("propName"))
    .def("floatProperty", (smtk::model::FloatList & (smtk::mesh::Resource::*)(::smtk::mesh::MeshSet const &, ::std::string const &)) &smtk::mesh::Resource::floatProperty, py::arg("meshset"), py::arg("propName"))
    .def("hasAssociations", &smtk::mesh::Resource::hasAssociations)
    .def("hasFloatProperty", &smtk::mesh::Resource::hasFloatProperty, py::arg("meshset"), py::arg("propName"))
    .def("hasIntegerProperty", &smtk::mesh::Resource::hasIntegerProperty, py::arg("meshset"), py::arg("propName"))
    .def("hasStringProperty", &smtk::mesh::Resource::hasStringProperty, py::arg("meshset"), py::arg("propName"))
    .def("integerProperty", (smtk::model::IntegerList const & (smtk::mesh::Resource::*)(::smtk::mesh::MeshSet const &, ::std::string const &) const) &smtk::mesh::Resource::integerProperty, py::arg("meshset"), py::arg("propName"))
    .def("integerProperty", (smtk::model::IntegerList & (smtk::mesh::Resource::*)(::smtk::mesh::MeshSet const &, ::std::string const &)) &smtk::mesh::Resource::integerProperty, py::arg("meshset"), py::arg("propName"))
    .def("interface", &smtk::mesh::Resource::interface)
    .def("interfaceName", &smtk::mesh::Resource::interfaceName)
    .def("isAssociatedToModel", &smtk::mesh::Resource::isAssociatedToModel)
    .def("isModified", &smtk::mesh::Resource::isModified)
    .def("isValid", &smtk::mesh::Resource::isValid)
    .def("meshNames", &smtk::mesh::Resource::meshNames)
    .def("meshes", (smtk::mesh::MeshSet (smtk::mesh::Resource::*)() const) &smtk::mesh::Resource::meshes)
    .def("meshes", (smtk::mesh::MeshSet (smtk::mesh::Resource::*)(::smtk::mesh::DimensionType) const) &smtk::mesh::Resource::meshes, py::arg("dim"))
    .def("meshes", (smtk::mesh::MeshSet (smtk::mesh::Resource::*)(::smtk::mesh::Domain const &) const) &smtk::mesh::Resource::meshes, py::arg("d"))
    .def("meshes", (smtk::mesh::MeshSet (smtk::mesh::Resource::*)(::smtk::mesh::Dirichlet const &) const) &smtk::mesh::Resource::meshes, py::arg("d"))
    .def("meshes", (smtk::mesh::MeshSet (smtk::mesh::Resource::*)(::smtk::mesh::Neumann const &) const) &smtk::mesh::Resource::meshes, py::arg("n"))
    .def("meshes", (smtk::mesh::MeshSet (smtk::mesh::Resource::*)(::std::string const &) const) &smtk::mesh::Resource::meshes, py::arg("name"))
    .def("name", (std::string (smtk::mesh::Resource::*)() const) &smtk::mesh::Resource::name)
    .def("name", (void (smtk::mesh::Resource::*)(::std::string const &)) &smtk::mesh::Resource::name, py::arg("n"))
    .def("neumannMeshes", &smtk::mesh::Resource::neumannMeshes, py::arg("n"))
    .def("neumanns", &smtk::mesh::Resource::neumanns)
    .def("numberOfMeshes", &smtk::mesh::Resource::numberOfMeshes)
    .def("pointConnectivity", &smtk::mesh::Resource::pointConnectivity)
    .def("points", &smtk::mesh::Resource::points)
    .def("readLocation", (const smtk::common::FileLocation& (smtk::mesh::Resource::*)() const) &smtk::mesh::Resource::readLocation)
    .def("removeFloatProperty", &smtk::mesh::Resource::removeFloatProperty, py::arg("meshset"), py::arg("propName"))
    .def("removeIntegerProperty", &smtk::mesh::Resource::removeIntegerProperty, py::arg("meshset"), py::arg("propName"))
    .def("removeMeshes", &smtk::mesh::Resource::removeMeshes, py::arg("meshesToDelete"))
    .def("removeStringProperty", &smtk::mesh::Resource::removeStringProperty, py::arg("meshset"), py::arg("propName"))
    .def("setAssociation", &smtk::mesh::Resource::setAssociation, py::arg("eref"), py::arg("meshset"))
    .def("setDirichletOnMeshes", &smtk::mesh::Resource::setDirichletOnMeshes, py::arg("meshes"), py::arg("d"))
    .def("setDomainOnMeshes", &smtk::mesh::Resource::setDomainOnMeshes, py::arg("meshes"), py::arg("m"))
    .def("setFloatProperty", (void (smtk::mesh::Resource::*)(::smtk::mesh::MeshSet const &, ::std::string const &, ::smtk::model::Float)) &smtk::mesh::Resource::setFloatProperty, py::arg("meshset"), py::arg("propName"), py::arg("propValue"))
    .def("setFloatProperty", (void (smtk::mesh::Resource::*)(::smtk::mesh::MeshSet const &, ::std::string const &, ::smtk::model::FloatList const &)) &smtk::mesh::Resource::setFloatProperty, py::arg("meshset"), py::arg("propName"), py::arg("propValue"))
    .def("setIntegerProperty", (void (smtk::mesh::Resource::*)(::smtk::mesh::MeshSet const &, ::std::string const &, ::smtk::model::Integer)) &smtk::mesh::Resource::setIntegerProperty, py::arg("meshset"), py::arg("propName"), py::arg("propValue"))
    .def("setIntegerProperty", (void (smtk::mesh::Resource::*)(::smtk::mesh::MeshSet const &, ::std::string const &, ::smtk::model::IntegerList const &)) &smtk::mesh::Resource::setIntegerProperty, py::arg("meshset"), py::arg("propName"), py::arg("propValue"))
    .def("setNeumannOnMeshes", &smtk::mesh::Resource::setNeumannOnMeshes, py::arg("meshes"), py::arg("n"))
    .def("setStringProperty", (void (smtk::mesh::Resource::*)(::smtk::mesh::MeshSet const &, ::std::string const &, ::smtk::model::String const &)) &smtk::mesh::Resource::setStringProperty, py::arg("meshset"), py::arg("propName"), py::arg("propValue"))
    .def("setStringProperty", (void (smtk::mesh::Resource::*)(::smtk::mesh::MeshSet const &, ::std::string const &, ::smtk::model::StringList const &)) &smtk::mesh::Resource::setStringProperty, py::arg("meshset"), py::arg("propName"), py::arg("propValue"))
    .def("stringProperty", (smtk::model::StringList const & (smtk::mesh::Resource::*)(::smtk::mesh::MeshSet const &, ::std::string const &) const) &smtk::mesh::Resource::stringProperty, py::arg("meshset"), py::arg("propName"))
    .def("stringProperty", (smtk::model::StringList & (smtk::mesh::Resource::*)(::smtk::mesh::MeshSet const &, ::std::string const &)) &smtk::mesh::Resource::stringProperty, py::arg("meshset"), py::arg("propName"))
    .def("types", &smtk::mesh::Resource::types)
    .def("writeLocation", (void (smtk::mesh::Resource::*)(::smtk::common::FileLocation const &)) &smtk::mesh::Resource::writeLocation, py::arg("path"))
    .def("writeLocation", (void (smtk::mesh::Resource::*)(::std::string const &)) &smtk::mesh::Resource::writeLocation, py::arg("path"))
    .def("writeLocation", (smtk::common::FileLocation const & (smtk::mesh::Resource::*)() const) &smtk::mesh::Resource::writeLocation)
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::Resource> i) {
        return std::dynamic_pointer_cast<smtk::mesh::Resource>(i);
      })
    ;
  return instance;
}

#endif
