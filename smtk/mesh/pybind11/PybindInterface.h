//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_Interface_h
#define pybind_smtk_mesh_Interface_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/Interface.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/mesh/CellTypes.h"
#include "smtk/mesh/Handle.h"
#include "smtk/mesh/PointConnectivity.h"
#include "smtk/mesh/TypeSet.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::Allocator > pybind11_init_smtk_mesh_Allocator(py::module &m)
{
  PySharedPtrClass< smtk::mesh::Allocator > instance(m, "Allocator");
  instance
    .def("deepcopy", (smtk::mesh::Allocator & (smtk::mesh::Allocator::*)(::smtk::mesh::Allocator const &)) &smtk::mesh::Allocator::operator=)
    // .def("allocateCells", &smtk::mesh::Allocator::allocateCells, py::arg("cellType"), py::arg("numCellsToAlloc"), py::arg("numVertsPerCell"), py::arg("createdCellIds"), py::arg("connectivityArray"))
    .def("allocatePoints", &smtk::mesh::Allocator::allocatePoints, py::arg("numPointsToAlloc"), py::arg("firstVertexHandle"), py::arg("coordinateMemory"))
    .def("connectivityModified", &smtk::mesh::Allocator::connectivityModified, py::arg("cellsToUpdate"), py::arg("numVertsPerCell"), py::arg("connectivityArray"))
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::BufferedCellAllocator > pybind11_init_smtk_mesh_BufferedCellAllocator(py::module &m)
{
  PySharedPtrClass< smtk::mesh::BufferedCellAllocator > instance(m, "BufferedCellAllocator");
  instance
    .def("deepcopy", (smtk::mesh::BufferedCellAllocator & (smtk::mesh::BufferedCellAllocator::*)(::smtk::mesh::BufferedCellAllocator const &)) &smtk::mesh::BufferedCellAllocator::operator=)
    .def("addCell", (bool (smtk::mesh::BufferedCellAllocator::*)(::smtk::mesh::CellType, long long int *, ::size_t)) &smtk::mesh::BufferedCellAllocator::addCell, py::arg("ctype"), py::arg("pointIds"), py::arg("nCoordinates") = 0)
    .def("addCell", (bool (smtk::mesh::BufferedCellAllocator::*)(::smtk::mesh::CellType, long int *, ::size_t)) &smtk::mesh::BufferedCellAllocator::addCell, py::arg("ctype"), py::arg("pointIds"), py::arg("nCoordinates") = 0)
    .def("addCell", (bool (smtk::mesh::BufferedCellAllocator::*)(::smtk::mesh::CellType, int *, ::size_t)) &smtk::mesh::BufferedCellAllocator::addCell, py::arg("ctype"), py::arg("pointIds"), py::arg("nCoordinates") = 0)
    .def("cells", &smtk::mesh::BufferedCellAllocator::cells)
    .def("flush", &smtk::mesh::BufferedCellAllocator::flush)
    .def("isValid", &smtk::mesh::BufferedCellAllocator::isValid)
    .def("reserveNumberOfCoordinates", &smtk::mesh::BufferedCellAllocator::reserveNumberOfCoordinates, py::arg("nCoordinates"))
    .def("setCoordinate", (bool (smtk::mesh::BufferedCellAllocator::*)(::size_t, double *)) &smtk::mesh::BufferedCellAllocator::setCoordinate, py::arg("coord"), py::arg("xyz"))
    .def("setCoordinate", (bool (smtk::mesh::BufferedCellAllocator::*)(::size_t, double, double, double)) &smtk::mesh::BufferedCellAllocator::setCoordinate, py::arg("coord"), py::arg("x"), py::arg("y"), py::arg("z"))
    .def("setCoordinate", (bool (smtk::mesh::BufferedCellAllocator::*)(::size_t, float *)) &smtk::mesh::BufferedCellAllocator::setCoordinate, py::arg("coord"), py::arg("xyz"))
    .def("setCoordinate", (bool (smtk::mesh::BufferedCellAllocator::*)(::size_t, float, float, float)) &smtk::mesh::BufferedCellAllocator::setCoordinate, py::arg("coord"), py::arg("x"), py::arg("y"), py::arg("z"))
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::ConnectivityStorage > pybind11_init_smtk_mesh_ConnectivityStorage(py::module &m)
{
  PySharedPtrClass< smtk::mesh::ConnectivityStorage > instance(m, "ConnectivityStorage");
  instance
    .def("deepcopy", (smtk::mesh::ConnectivityStorage & (smtk::mesh::ConnectivityStorage::*)(::smtk::mesh::ConnectivityStorage const &)) &smtk::mesh::ConnectivityStorage::operator=)
    .def("cellSize", &smtk::mesh::ConnectivityStorage::cellSize)
    .def("equal", &smtk::mesh::ConnectivityStorage::equal, py::arg("other"))
    // .def("fetchNextCell", &smtk::mesh::ConnectivityStorage::fetchNextCell, py::arg("state"), py::arg("cellType"), py::arg("numPts"), py::arg("points"))
    .def("initTraversal", &smtk::mesh::ConnectivityStorage::initTraversal, py::arg("state"))
    .def("vertSize", &smtk::mesh::ConnectivityStorage::vertSize)
    ;
  py::class_< smtk::mesh::ConnectivityStorage::IterationState >(instance, "IterationState")
    .def(py::init<>())
    .def(py::init<::smtk::mesh::ConnectivityStorage::IterationState const &>())
    .def("deepcopy", (smtk::mesh::ConnectivityStorage::IterationState & (smtk::mesh::ConnectivityStorage::IterationState::*)(::smtk::mesh::ConnectivityStorage::IterationState const &)) &smtk::mesh::ConnectivityStorage::IterationState::operator=)
    .def_readwrite("whichConnectivityVector", &smtk::mesh::ConnectivityStorage::IterationState::whichConnectivityVector)
    .def_readwrite("ptrOffsetInVector", &smtk::mesh::ConnectivityStorage::IterationState::ptrOffsetInVector)
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::IncrementalAllocator > pybind11_init_smtk_mesh_IncrementalAllocator(py::module &m)
{
  PySharedPtrClass< smtk::mesh::IncrementalAllocator > instance(m, "IncrementalAllocator");
  instance
    .def("deepcopy", (smtk::mesh::IncrementalAllocator & (smtk::mesh::IncrementalAllocator::*)(::smtk::mesh::IncrementalAllocator const &)) &smtk::mesh::IncrementalAllocator::operator=)
    .def("addCell", (bool (smtk::mesh::IncrementalAllocator::*)(::smtk::mesh::CellType, long long int *, ::size_t)) &smtk::mesh::IncrementalAllocator::addCell, py::arg("ctype"), py::arg("pointIds"), py::arg("nCoordinates") = 0)
    .def("addCell", (bool (smtk::mesh::IncrementalAllocator::*)(::smtk::mesh::CellType, long int *, ::size_t)) &smtk::mesh::IncrementalAllocator::addCell, py::arg("ctype"), py::arg("pointIds"), py::arg("nCoordinates") = 0)
    .def("addCell", (bool (smtk::mesh::IncrementalAllocator::*)(::smtk::mesh::CellType, int *, ::size_t)) &smtk::mesh::IncrementalAllocator::addCell, py::arg("ctype"), py::arg("pointIds"), py::arg("nCoordinates") = 0)
    .def("addCoordinate", (size_t (smtk::mesh::IncrementalAllocator::*)(double *)) &smtk::mesh::IncrementalAllocator::addCoordinate, py::arg("xyz"))
    .def("addCoordinate", (size_t (smtk::mesh::IncrementalAllocator::*)(double, double, double)) &smtk::mesh::IncrementalAllocator::addCoordinate, py::arg("x"), py::arg("y"), py::arg("z"))
    .def("addCoordinate", (size_t (smtk::mesh::IncrementalAllocator::*)(float *)) &smtk::mesh::IncrementalAllocator::addCoordinate, py::arg("xyz"))
    .def("addCoordinate", (size_t (smtk::mesh::IncrementalAllocator::*)(float, float, float)) &smtk::mesh::IncrementalAllocator::addCoordinate, py::arg("x"), py::arg("y"), py::arg("z"))
    .def("cells", &smtk::mesh::IncrementalAllocator::cells)
    .def("flush", &smtk::mesh::IncrementalAllocator::flush)
    .def("isValid", &smtk::mesh::IncrementalAllocator::isValid)
    .def("setCoordinate", (bool (smtk::mesh::IncrementalAllocator::*)(::size_t, double *)) &smtk::mesh::IncrementalAllocator::setCoordinate, py::arg("coord"), py::arg("xyz"))
    .def("setCoordinate", (bool (smtk::mesh::IncrementalAllocator::*)(::size_t, double, double, double)) &smtk::mesh::IncrementalAllocator::setCoordinate, py::arg("coord"), py::arg("x"), py::arg("y"), py::arg("z"))
    .def("setCoordinate", (bool (smtk::mesh::IncrementalAllocator::*)(::size_t, float *)) &smtk::mesh::IncrementalAllocator::setCoordinate, py::arg("coord"), py::arg("xyz"))
    .def("setCoordinate", (bool (smtk::mesh::IncrementalAllocator::*)(::size_t, float, float, float)) &smtk::mesh::IncrementalAllocator::setCoordinate, py::arg("coord"), py::arg("x"), py::arg("y"), py::arg("z"))
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::Interface > pybind11_init_smtk_mesh_Interface(py::module &m)
{
  PySharedPtrClass< smtk::mesh::Interface > instance(m, "Interface");
  instance
    .def("deepcopy", (smtk::mesh::Interface & (smtk::mesh::Interface::*)(::smtk::mesh::Interface const &)) &smtk::mesh::Interface::operator=)
    .def("allocator", &smtk::mesh::Interface::allocator)
    .def("bufferedCellAllocator", &smtk::mesh::Interface::bufferedCellAllocator)
    .def("cellForEach", &smtk::mesh::Interface::cellForEach, py::arg("cells"), py::arg("a"), py::arg("filter"))
    .def("computeCellFieldTags", &smtk::mesh::Interface::computeCellFieldTags, py::arg("handle"))
    .def("computePointFieldTags", &smtk::mesh::Interface::computePointFieldTags, py::arg("handle"))
    .def("computeDirichletValues", &smtk::mesh::Interface::computeDirichletValues, py::arg("meshsets"))
    .def("computeDomainValues", &smtk::mesh::Interface::computeDomainValues, py::arg("meshsets"))
    .def("computeModelEntities", &smtk::mesh::Interface::computeModelEntities, py::arg("meshsets"))
    .def("computeNames", &smtk::mesh::Interface::computeNames, py::arg("meshsets"))
    .def("computeNeumannValues", &smtk::mesh::Interface::computeNeumannValues, py::arg("meshsets"))
    .def("computeShell", &smtk::mesh::Interface::computeShell, py::arg("meshes"), py::arg("shell"))
    .def("computeTypes", &smtk::mesh::Interface::computeTypes, py::arg("range"))
    .def("connectivityStorage", &smtk::mesh::Interface::connectivityStorage, py::arg("cells"))
    .def("createCellField", &smtk::mesh::Interface::createCellField, py::arg("meshsets"), py::arg("name"), py::arg("dimension"), py::arg("field"))
    .def("createPointField", &smtk::mesh::Interface::createPointField, py::arg("meshsets"), py::arg("name"), py::arg("dimension"), py::arg("field"))
    .def("createMesh", &smtk::mesh::Interface::createMesh, py::arg("cells"), py::arg("meshHandle"))
    .def("deleteCellField", &smtk::mesh::Interface::deleteCellField, py::arg("cfTag"), py::arg("meshsets"))
    .def("deletePointField", &smtk::mesh::Interface::deletePointField, py::arg("cfTag"), py::arg("meshsets"))
    .def("deleteHandles", &smtk::mesh::Interface::deleteHandles, py::arg("toDel"))
    .def("findAssociations", &smtk::mesh::Interface::findAssociations, py::arg("root"), py::arg("modelUUID"))
    .def("getCells", (smtk::mesh::HandleRange (smtk::mesh::Interface::*)(::smtk::mesh::HandleRange const &) const) &smtk::mesh::Interface::getCells, py::arg("meshsets"))
    .def("getCells", (smtk::mesh::HandleRange (smtk::mesh::Interface::*)(::smtk::mesh::HandleRange const &, ::smtk::mesh::CellType) const) &smtk::mesh::Interface::getCells, py::arg("meshsets"), py::arg("cellType"))
    .def("getCells", (smtk::mesh::HandleRange (smtk::mesh::Interface::*)(::smtk::mesh::HandleRange const &, ::smtk::mesh::CellTypes const &) const) &smtk::mesh::Interface::getCells, py::arg("meshsets"), py::arg("cellTypes"))
    .def("getCells", (smtk::mesh::HandleRange (smtk::mesh::Interface::*)(::smtk::mesh::HandleRange const &, ::smtk::mesh::DimensionType) const) &smtk::mesh::Interface::getCells, py::arg("meshsets"), py::arg("dim"))
    .def("getCoordinates", (bool (smtk::mesh::Interface::*)(::smtk::mesh::HandleRange const &, double *) const) &smtk::mesh::Interface::getCoordinates, py::arg("points"), py::arg("xyz"))
    .def("getCoordinates", (bool (smtk::mesh::Interface::*)(::smtk::mesh::HandleRange const &, float *) const) &smtk::mesh::Interface::getCoordinates, py::arg("points"), py::arg("xyz"))
    .def("getField", (bool (smtk::mesh::Interface::*)(const smtk::mesh::HandleRange&, const smtk::mesh::CellFieldTag&, double*) const) &smtk::mesh::Interface::getField, py::arg("cells"), py::arg("cfTag"), py::arg("field"))
    .def("getField", (bool (smtk::mesh::Interface::*)(const smtk::mesh::HandleRange&, const smtk::mesh::PointFieldTag&, double*) const) &smtk::mesh::Interface::getField, py::arg("points"), py::arg("pfTag"), py::arg("field"))
    .def("getCellField", &smtk::mesh::Interface::getCellField, py::arg("meshsets"), py::arg("cfTag"), py::arg("field"))
    .def("getCellFieldDimension", &smtk::mesh::Interface::getCellFieldDimension, py::arg("cfTag"))
    .def("getMeshsets", (smtk::mesh::HandleRange (smtk::mesh::Interface::*)(::smtk::mesh::Handle) const) &smtk::mesh::Interface::getMeshsets, py::arg("handle"))
    .def("getMeshsets", (smtk::mesh::HandleRange (smtk::mesh::Interface::*)(::smtk::mesh::Handle, int) const) &smtk::mesh::Interface::getMeshsets, py::arg("handle"), py::arg("dimension"))
    .def("getMeshsets", (smtk::mesh::HandleRange (smtk::mesh::Interface::*)(::smtk::mesh::Handle, ::std::string const &) const) &smtk::mesh::Interface::getMeshsets, py::arg("handle"), py::arg("name"))
    .def("getMeshsets", (smtk::mesh::HandleRange (smtk::mesh::Interface::*)(::smtk::mesh::Handle, ::smtk::mesh::Domain const &) const) &smtk::mesh::Interface::getMeshsets, py::arg("handle"), py::arg("domain"))
    .def("getMeshsets", (smtk::mesh::HandleRange (smtk::mesh::Interface::*)(::smtk::mesh::Handle, ::smtk::mesh::Dirichlet const &) const) &smtk::mesh::Interface::getMeshsets, py::arg("handle"), py::arg("dirichlet"))
    .def("getMeshsets", (smtk::mesh::HandleRange (smtk::mesh::Interface::*)(::smtk::mesh::Handle, ::smtk::mesh::Neumann const &) const) &smtk::mesh::Interface::getMeshsets, py::arg("handle"), py::arg("neumann"))
    .def("getMeshsets", (smtk::mesh::HandleRange (smtk::mesh::Interface::*)(::smtk::mesh::Handle, ::smtk::mesh::CellFieldTag const &) const) &smtk::mesh::Interface::getMeshsets, py::arg("handle"), py::arg("cfTag"))
    .def("getMeshsets", (smtk::mesh::HandleRange (smtk::mesh::Interface::*)(::smtk::mesh::Handle, ::smtk::mesh::PointFieldTag const &) const) &smtk::mesh::Interface::getMeshsets, py::arg("handle"), py::arg("pfTag"))
    .def("getPointField", &smtk::mesh::Interface::getPointField, py::arg("meshsets"), py::arg("cfTag"), py::arg("field"))
    .def("getPointFieldDimension", &smtk::mesh::Interface::getPointFieldDimension, py::arg("cfTag"))
    .def("getPoints", &smtk::mesh::Interface::getPoints, py::arg("cells"))
    .def("getRoot", &smtk::mesh::Interface::getRoot)
    .def("hasCellField", &smtk::mesh::Interface::hasCellField, py::arg("meshsets"), py::arg("cfTag"))
    .def("hasPointField", &smtk::mesh::Interface::hasPointField, py::arg("meshsets"), py::arg("cfTag"))
    .def("incrementalAllocator", &smtk::mesh::Interface::incrementalAllocator)
    .def("isModified", &smtk::mesh::Interface::isModified)
    .def("mergeCoincidentContactPoints", &smtk::mesh::Interface::mergeCoincidentContactPoints, py::arg("meshes"), py::arg("tolerance"))
    .def("meshForEach", &smtk::mesh::Interface::meshForEach, py::arg("meshes"), py::arg("filter"))
    .def("name", &smtk::mesh::Interface::name)
    .def("numMeshes", &smtk::mesh::Interface::numMeshes, py::arg("handle"))
    .def("pointDifference", &smtk::mesh::Interface::pointDifference, py::arg("a"), py::arg("b"), py::arg("bpc"), py::arg("containsFunctor"))
    .def("pointForEach", &smtk::mesh::Interface::pointForEach, py::arg("points"), py::arg("filter"))
    .def("pointIntersect", &smtk::mesh::Interface::pointIntersect, py::arg("a"), py::arg("b"), py::arg("bpc"), py::arg("containsFunctor"))
    .def("pointLocator", (smtk::mesh::PointLocatorImplPtr (smtk::mesh::Interface::*)(::smtk::mesh::HandleRange const &)) &smtk::mesh::Interface::pointLocator, py::arg("points"))
    .def("pointLocator", (smtk::mesh::PointLocatorImplPtr (smtk::mesh::Interface::*)(double const * const, ::size_t, bool)) &smtk::mesh::Interface::pointLocator, py::arg("xyzs"), py::arg("numPoints"), py::arg("ignoreZValues") = false)
    .def("pointLocator", (smtk::mesh::PointLocatorImplPtr (smtk::mesh::Interface::*)(float const * const, ::size_t, bool)) &smtk::mesh::Interface::pointLocator, py::arg("xyzs"), py::arg("numPoints"), py::arg("ignoreZValues") = false)
    .def("rangeDifference", &smtk::mesh::Interface::rangeDifference, py::arg("a"), py::arg("b"))
    .def("rangeIntersect", &smtk::mesh::Interface::rangeIntersect, py::arg("a"), py::arg("b"))
    .def("rangeUnion", &smtk::mesh::Interface::rangeUnion, py::arg("a"), py::arg("b"))
    .def("rootAssociation", &smtk::mesh::Interface::rootAssociation)
    .def("setAssociation", &smtk::mesh::Interface::setAssociation, py::arg("modelUUID"), py::arg("meshsets"))
    .def("setCoordinates", (bool (smtk::mesh::Interface::*)(::smtk::mesh::HandleRange const &, double const * const)) &smtk::mesh::Interface::setCoordinates, py::arg("points"), py::arg("xyz"))
    .def("setCoordinates", (bool (smtk::mesh::Interface::*)(::smtk::mesh::HandleRange const &, float const * const)) &smtk::mesh::Interface::setCoordinates, py::arg("points"), py::arg("xyz"))
    .def("setField", (bool (smtk::mesh::Interface::*)(const smtk::mesh::HandleRange&, const smtk::mesh::PointFieldTag&, const double* const)) &smtk::mesh::Interface::setField, py::arg("points"), py::arg("pfTag"), py::arg("field"))
    .def("setField", (bool (smtk::mesh::Interface::*)(const smtk::mesh::HandleRange&, const smtk::mesh::CellFieldTag&, const double* const)) &smtk::mesh::Interface::setField, py::arg("points"), py::arg("pfTag"), py::arg("field"))
    .def("setCellField", &smtk::mesh::Interface::setCellField, py::arg("meshsets"), py::arg("cfTag"), py::arg("field"))
    .def("setDirichlet", &smtk::mesh::Interface::setDirichlet, py::arg("meshsets"), py::arg("dirichlet"))
    .def("setDomain", &smtk::mesh::Interface::setDomain, py::arg("meshsets"), py::arg("domain"))
    .def("setModifiedState", &smtk::mesh::Interface::setModifiedState, py::arg("state"))
    .def("setNeumann", &smtk::mesh::Interface::setNeumann, py::arg("meshsets"), py::arg("neumann"))
    .def("setPointField", &smtk::mesh::Interface::setPointField, py::arg("meshsets"), py::arg("pfTag"), py::arg("field"))
    .def("setRootAssociation", &smtk::mesh::Interface::setRootAssociation, py::arg("modelUUID"))
    ;
  return instance;
}

PySharedPtrClass< smtk::mesh::PointLocatorImpl > pybind11_init_smtk_mesh_PointLocatorImpl(py::module &m)
{
  PySharedPtrClass< smtk::mesh::PointLocatorImpl > instance(m, "PointLocatorImpl");
  instance
    .def("deepcopy", (smtk::mesh::PointLocatorImpl & (smtk::mesh::PointLocatorImpl::*)(::smtk::mesh::PointLocatorImpl const &)) &smtk::mesh::PointLocatorImpl::operator=)
    .def("locatePointsWithinRadius", &smtk::mesh::PointLocatorImpl::locatePointsWithinRadius, py::arg("x"), py::arg("y"), py::arg("z"), py::arg("radius"), py::arg("results"))
    .def("range", &smtk::mesh::PointLocatorImpl::range)
    ;
  py::class_< smtk::mesh::PointLocatorImpl::Results >(instance, "Results")
    .def(py::init<>())
    .def(py::init<::smtk::mesh::PointLocatorImpl::Results const &>())
    .def("deepcopy", (smtk::mesh::PointLocatorImpl::Results & (smtk::mesh::PointLocatorImpl::Results::*)(::smtk::mesh::PointLocatorImpl::Results const &)) &smtk::mesh::PointLocatorImpl::Results::operator=)
    .def_readwrite("pointIds", &smtk::mesh::PointLocatorImpl::Results::pointIds)
    .def_readwrite("sqDistances", &smtk::mesh::PointLocatorImpl::Results::sqDistances)
    .def_readwrite("x_s", &smtk::mesh::PointLocatorImpl::Results::x_s)
    .def_readwrite("y_s", &smtk::mesh::PointLocatorImpl::Results::y_s)
    .def_readwrite("z_s", &smtk::mesh::PointLocatorImpl::Results::z_s)
    .def_readwrite("want_sqDistances", &smtk::mesh::PointLocatorImpl::Results::want_sqDistances)
    .def_readwrite("want_Coordinates", &smtk::mesh::PointLocatorImpl::Results::want_Coordinates)
    ;
  return instance;
}

#endif
