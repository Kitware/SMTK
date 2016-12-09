//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_ForEachTypes_h
#define pybind_smtk_mesh_ForEachTypes_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/mesh/ForEachTypes.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/MeshSet.h"

namespace py = pybind11;

class PyMeshForEach : public smtk::mesh::MeshForEach
{
public:
  using smtk::mesh::MeshForEach::MeshForEach;
  void forMesh(smtk::mesh::MeshSet& singleMesh) override
  {
    PYBIND11_OVERLOAD_PURE(void, smtk::mesh::MeshForEach, forMesh, singleMesh);
  }
};

class PyCellForEach : public smtk::mesh::CellForEach
{
public:
  using smtk::mesh::CellForEach::CellForEach;
  void forCell(const smtk::mesh::Handle& cellId,
               smtk::mesh::CellType cellType,
               int numPointIds) override
  {
    PYBIND11_OVERLOAD_PURE(void, smtk::mesh::CellForEach, forCell, cellId, cellType, numPointIds);
  }
};

class PyPointForEach : public smtk::mesh::PointForEach
{
public:
  using smtk::mesh::PointForEach::PointForEach;
  void forPoints(const smtk::mesh::HandleRange& pointIds,
                 std::vector<double>& xyz,
                 bool& coordinatesModified) override
  {
    PYBIND11_OVERLOAD_PURE(void, smtk::mesh::PointForEach, forPoints, pointIds, xyz, coordinatesModified);
  }
};

PySharedPtrClass< smtk::mesh::MeshForEach > pybind11_init_smtk_mesh_MeshForEach(py::module &m)
{
  py::class_<smtk::mesh::MeshForEach, std::shared_ptr<smtk::mesh::MeshForEach>, PyMeshForEach > instance(m, "MeshForEach");
  instance
    .def(py::init<>())
    .def("deepcopy", (smtk::mesh::MeshForEach & (smtk::mesh::MeshForEach::*)(::smtk::mesh::MeshForEach const &)) &smtk::mesh::MeshForEach::operator=)
    .def("forMesh", &smtk::mesh::MeshForEach::forMesh, py::arg("singleMesh"))
    .def_readwrite("m_collection", &smtk::mesh::MeshForEach::m_collection)
    ;

  return instance;
}

PySharedPtrClass< smtk::mesh::CellForEach > pybind11_init_smtk_mesh_CellForEach(py::module &m)
{
  py::class_<smtk::mesh::CellForEach, std::shared_ptr<smtk::mesh::CellForEach>, PyCellForEach > instance(m, "CellForEach");
  instance
    .def(py::init<>())
    .def(py::init<bool>())
    .def("deepcopy", (smtk::mesh::CellForEach & (smtk::mesh::CellForEach::*)(::smtk::mesh::CellForEach const &)) &smtk::mesh::CellForEach::operator=)
    .def("collection", (smtk::mesh::CollectionPtr (smtk::mesh::CellForEach::*)() const) &smtk::mesh::CellForEach::collection)
    .def("collection", (void (smtk::mesh::CellForEach::*)(::smtk::mesh::CollectionPtr)) &smtk::mesh::CellForEach::collection, py::arg("c"))
    .def("coordinates", (std::vector<double, std::allocator<double> > const & (smtk::mesh::CellForEach::*)() const) &smtk::mesh::CellForEach::coordinates)
    .def("coordinates", (void (smtk::mesh::CellForEach::*)(::std::vector<double, std::allocator<double> > *)) &smtk::mesh::CellForEach::coordinates, py::arg("coords"))
    .def("forCell", &smtk::mesh::CellForEach::forCell, py::arg("cellId"), py::arg("cellType"), py::arg("numPointIds"))
    .def("pointId", &smtk::mesh::CellForEach::pointId, py::arg("index"))
    .def("pointIds", (smtk::mesh::Handle const * (smtk::mesh::CellForEach::*)() const) &smtk::mesh::CellForEach::pointIds)
    .def("pointIds", (void (smtk::mesh::CellForEach::*)(::smtk::mesh::Handle const *)) &smtk::mesh::CellForEach::pointIds, py::arg("ptIds"))
    .def("wantsCoordinates", &smtk::mesh::CellForEach::wantsCoordinates)
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::PointForEach > pybind11_init_smtk_mesh_PointForEach(py::module &m)
{
  py::class_<smtk::mesh::PointForEach, std::shared_ptr<smtk::mesh::PointForEach>, PyPointForEach > instance(m, "PointForEach");
  instance
    .def(py::init<>())
    .def("deepcopy", (smtk::mesh::PointForEach & (smtk::mesh::PointForEach::*)(::smtk::mesh::PointForEach const &)) &smtk::mesh::PointForEach::operator=)
    .def("forPoints", &smtk::mesh::PointForEach::forPoints, py::arg("pointIds"), py::arg("xyz"), py::arg("coordinatesModified"))
    .def_readwrite("m_collection", &smtk::mesh::PointForEach::m_collection)
    ;
  return instance;
}

#endif
