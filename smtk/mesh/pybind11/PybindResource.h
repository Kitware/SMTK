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
#include "smtk/resource/Resource.h"

#include "smtk/common/UUID.h"

#include "smtk/model/EntityIterator.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::mesh::Resource> pybind11_init_smtk_mesh_Resource(py::module &m)
{
  PySharedPtrClass< smtk::mesh::Resource, smtk::resource::Resource > instance(m, "Resource");
  instance
    .def_property("modelResource", &smtk::mesh::Resource::modelResource, &smtk::mesh::Resource::setModelResource)
    .def("associateToModel", &smtk::mesh::Resource::associateToModel, py::arg("uuid"))
    .def("associatedModel", &smtk::mesh::Resource::associatedModel)
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)() const) &smtk::mesh::Resource::cells)
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::mesh::CellType) const) &smtk::mesh::Resource::cells, py::arg("cellType"))
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::mesh::CellTypes) const) &smtk::mesh::Resource::cells, py::arg("cellTypes"))
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::Resource::*)(::smtk::mesh::DimensionType) const) &smtk::mesh::Resource::cells, py::arg("dim"))
    .def("classifiedTo", &smtk::mesh::Resource::classifiedTo)
    .def("classifyTo", &smtk::mesh::Resource::classifyTo)
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
    .def("hasAssociations", &smtk::mesh::Resource::hasAssociations)
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
    .def("neumannMeshes", &smtk::mesh::Resource::neumannMeshes, py::arg("n"))
    .def("neumanns", &smtk::mesh::Resource::neumanns)
    .def("numberOfMeshes", &smtk::mesh::Resource::numberOfMeshes)
    .def("pointConnectivity", &smtk::mesh::Resource::pointConnectivity)
    .def("points", &smtk::mesh::Resource::points)
    .def("readLocation", (const smtk::common::FileLocation& (smtk::mesh::Resource::*)() const) &smtk::mesh::Resource::readLocation)
    .def("removeMeshes", &smtk::mesh::Resource::removeMeshes, py::arg("meshesToDelete"))
    .def("setAssociation", &smtk::mesh::Resource::setAssociation, py::arg("eref"), py::arg("meshset"))
    .def("setDirichletOnMeshes", &smtk::mesh::Resource::setDirichletOnMeshes, py::arg("meshes"), py::arg("d"))
    .def("setDomainOnMeshes", &smtk::mesh::Resource::setDomainOnMeshes, py::arg("meshes"), py::arg("m"))
    .def("setNeumannOnMeshes", &smtk::mesh::Resource::setNeumannOnMeshes, py::arg("meshes"), py::arg("n"))
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
