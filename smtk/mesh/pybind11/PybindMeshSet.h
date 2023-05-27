//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_MeshSet_h
#define pybind_smtk_mesh_MeshSet_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/core/MeshSet.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/mesh/core/CellField.h"
#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/CellTypes.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/core/Handle.h"
#include "smtk/mesh/core/PointField.h"
#include "smtk/mesh/core/PointConnectivity.h"
#include "smtk/mesh/core/PointSet.h"
#include "smtk/mesh/core/TypeSet.h"
#include "smtk/model/EntityRef.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::mesh::MeshSet > pybind11_init_smtk_mesh_MeshSet(py::module &m)
{
  PySharedPtrClass< smtk::mesh::MeshSet > instance(m, "MeshSet");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::mesh::ResourcePtr const &, ::smtk::mesh::Handle>())
    .def(py::init<::smtk::mesh::ConstResourcePtr const &, ::smtk::mesh::Handle>())
    .def(py::init<::smtk::mesh::ResourcePtr const &, ::smtk::mesh::Handle, ::smtk::mesh::HandleRange const &>())
    .def(py::init<::smtk::mesh::ConstResourcePtr const &, ::smtk::mesh::Handle, ::smtk::mesh::HandleRange const &>())
    .def(py::init<::smtk::mesh::MeshSet const &>())
    .def("__ne__", (bool (smtk::mesh::MeshSet::*)(::smtk::mesh::MeshSet const &) const) &smtk::mesh::MeshSet::operator!=)
    .def("__lt__", (bool (smtk::mesh::MeshSet::*)(::smtk::mesh::MeshSet const &) const) &smtk::mesh::MeshSet::operator<)
    .def("deepcopy", (smtk::mesh::MeshSet & (smtk::mesh::MeshSet::*)(::smtk::mesh::MeshSet const &)) &smtk::mesh::MeshSet::operator=)
    .def("__eq__", (bool (smtk::mesh::MeshSet::*)(::smtk::mesh::MeshSet const &) const) &smtk::mesh::MeshSet::operator==)
    .def("append", &smtk::mesh::MeshSet::append, py::arg("other"))
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::MeshSet::*)() const) &smtk::mesh::MeshSet::cells)
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::MeshSet::*)(::smtk::mesh::CellType) const) &smtk::mesh::MeshSet::cells, py::arg("cellType"))
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::MeshSet::*)(::smtk::mesh::CellTypes) const) &smtk::mesh::MeshSet::cells, py::arg("cTypes"))
    .def("cells", (smtk::mesh::CellSet (smtk::mesh::MeshSet::*)(::smtk::mesh::DimensionType) const) &smtk::mesh::MeshSet::cells, py::arg("dim"))
    .def("resource", &smtk::mesh::MeshSet::resource)
    .def("createCellField", [](smtk::mesh::MeshSet& ms, const std::string& name, int dimension, const std::vector<double>& data){ return ms.createCellField(name, dimension, smtk::mesh::FieldType::Double, data.data()); }, py::arg("name"), py::arg("dimension"), py::arg("field"))
    .def("createCellField", [](smtk::mesh::MeshSet& ms, const std::string& name, int dimension, const std::vector<int>& data){ return ms.createCellField(name, dimension, smtk::mesh::FieldType::Integer, data.data()); }, py::arg("name"), py::arg("dimension"), py::arg("field"))
    .def("createCellField", [](smtk::mesh::MeshSet& ms, const std::string& name, int dimension, const smtk::mesh::FieldType& type){ return ms.createCellField(name, dimension, type); }, py::arg("name"), py::arg("dimension"), py::arg("type"))
    .def("cellField", &smtk::mesh::MeshSet::cellField, py::arg("name"))
    .def("cellFields", [](smtk::mesh::MeshSet const& ms) { std::vector<smtk::mesh::CellField> tmp; auto fields = ms.cellFields(); tmp.reserve(fields.size()); tmp.insert(tmp.begin(), fields.begin(), fields.end()); return tmp; })
    .def("createPointField", [](smtk::mesh::MeshSet& ms, const std::string& name, int dimension, const std::vector<double>& data){ return ms.createPointField(name, dimension, smtk::mesh::FieldType::Double, data.data()); }, py::arg("name"), py::arg("dimension"), py::arg("field"))
    .def("createPointField", [](smtk::mesh::MeshSet& ms, const std::string& name, int dimension, const std::vector<int>& data){ return ms.createPointField(name, dimension, smtk::mesh::FieldType::Integer, data.data()); }, py::arg("name"), py::arg("dimension"), py::arg("field"))
    .def("createPointField", [](smtk::mesh::MeshSet& ms, const std::string& name, int dimension, const smtk::mesh::FieldType& type){ return ms.createPointField(name, dimension, type); }, py::arg("name"), py::arg("dimension"), py::arg("type"))
    .def("pointField", &smtk::mesh::MeshSet::pointField, py::arg("name"))
    .def("pointFields", [](smtk::mesh::MeshSet const& ms) { std::vector<smtk::mesh::PointField> tmp; auto fields = ms.pointFields(); tmp.reserve(fields.size()); tmp.insert(tmp.begin(), fields.begin(), fields.end()); return tmp; })
    .def("dirichlets", &smtk::mesh::MeshSet::dirichlets)
    .def("domains", &smtk::mesh::MeshSet::domains)
    .def("extractShell", (smtk::mesh::MeshSet (smtk::mesh::MeshSet::*)() const) &smtk::mesh::MeshSet::extractShell)
    .def("extractAdjacenciesOfDimension", (smtk::mesh::MeshSet (smtk::mesh::MeshSet::*)(int) const) &smtk::mesh::MeshSet::extractAdjacenciesOfDimension)
    .def("id", &smtk::mesh::MeshSet::id)
    .def("is_empty", &smtk::mesh::MeshSet::is_empty)
    .def("mergeCoincidentContactPoints", &smtk::mesh::MeshSet::mergeCoincidentContactPoints, py::arg("tolerance") = 9.9999999999999995E-7)
    .def("modelEntities", (bool (smtk::mesh::MeshSet::*)(smtk::model::EntityRefArray&)) &smtk::mesh::MeshSet::modelEntities, py::arg("arg0"))
    .def("modelEntities", [](const smtk::mesh::MeshSet& ms){ smtk::model::EntityRefArray refs; ms.modelEntities(refs); return refs; })
    .def("modelEntityIds", &smtk::mesh::MeshSet::modelEntityIds)
    .def("name", &smtk::mesh::MeshSet::name)
    .def("names", &smtk::mesh::MeshSet::names)
    .def("neumanns", &smtk::mesh::MeshSet::neumanns)
    .def("pointConnectivity", &smtk::mesh::MeshSet::pointConnectivity)
    .def("points", &smtk::mesh::MeshSet::points, py::arg("boundary_only") = false)
    .def("range", &smtk::mesh::MeshSet::range)
    .def("removeCellField", &smtk::mesh::MeshSet::removeCellField, py::arg("cellfield"))
    .def("removePointField", &smtk::mesh::MeshSet::removePointField, py::arg("pointfield"))
    .def("setDirichlet", &smtk::mesh::MeshSet::setDirichlet, py::arg("d"))
    .def("setDomain", &smtk::mesh::MeshSet::setDomain, py::arg("d"))
    .def("setModelEntity", &smtk::mesh::MeshSet::setModelEntity, py::arg("arg0"))
    .def("setModelEntityId", &smtk::mesh::MeshSet::setModelEntityId, py::arg("arg0"))
    .def("setName", &smtk::mesh::MeshSet::setName)
    .def("setNeumann", &smtk::mesh::MeshSet::setNeumann, py::arg("n"))
    .def("size", &smtk::mesh::MeshSet::size)
    .def("subset", (smtk::mesh::MeshSet (smtk::mesh::MeshSet::*)(::smtk::mesh::DimensionType) const) &smtk::mesh::MeshSet::subset, py::arg("dim"))
    .def("subset", (smtk::mesh::MeshSet (smtk::mesh::MeshSet::*)(::smtk::mesh::Domain const &) const) &smtk::mesh::MeshSet::subset, py::arg("d"))
    .def("subset", (smtk::mesh::MeshSet (smtk::mesh::MeshSet::*)(::smtk::mesh::Dirichlet const &) const) &smtk::mesh::MeshSet::subset, py::arg("d"))
    .def("subset", (smtk::mesh::MeshSet (smtk::mesh::MeshSet::*)(::smtk::mesh::Neumann const &) const) &smtk::mesh::MeshSet::subset, py::arg("n"))
    .def("subset", (smtk::mesh::MeshSet (smtk::mesh::MeshSet::*)(::size_t) const) &smtk::mesh::MeshSet::subset, py::arg("ith"))
    .def("types", &smtk::mesh::MeshSet::types)
    ;
  return instance;
}

inline void pybind11_init_smtk_mesh_mesh_for_each(py::module &m)
{
  m.def("for_each", (void (*)(const smtk::mesh::MeshSet&, smtk::mesh::MeshForEach&)) &smtk::mesh::for_each, "", py::arg("a"), py::arg("filter"));
}

inline void pybind11_init_smtk_mesh_mesh_set_difference(py::module &m)
{
  m.def("set_difference", (smtk::mesh::MeshSet (*)(const smtk::mesh::MeshSet&, const smtk::mesh::MeshSet&)) &smtk::mesh::set_difference, "", py::arg("a"), py::arg("b"));
}

inline void pybind11_init_smtk_mesh_mesh_set_intersect(py::module &m)
{
  m.def("set_intersect", (smtk::mesh::MeshSet (*)(const smtk::mesh::MeshSet&, const smtk::mesh::MeshSet&)) &smtk::mesh::set_intersect, "", py::arg("a"), py::arg("b"));
}

inline void pybind11_init_smtk_mesh_mesh_set_union(py::module &m)
{
  m.def("set_union", (smtk::mesh::MeshSet (*)(const smtk::mesh::MeshSet&, const smtk::mesh::MeshSet&)) &smtk::mesh::set_union, "", py::arg("a"), py::arg("b"));
}

#endif
