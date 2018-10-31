//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindApplyToMesh.h"
#include "PybindCellField.h"
#include "PybindCellSet.h"
#include "PybindCellTypes.h"
#include "PybindCollection.h"
#include "PybindComponent.h"
#include "PybindDimensionTypes.h"
#include "PybindExtractMeshConstants.h"
#include "PybindExtractTessellation.h"
#include "PybindFieldTypes.h"
#include "PybindForEachTypes.h"
#include "PybindHandle.h"
#include "PybindInterface.h"
#include "PybindMeshSet.h"
#include "PybindMetrics.h"
#include "PybindPointField.h"
#include "PybindPointConnectivity.h"
#include "PybindPointLocator.h"
#include "PybindPointSet.h"
#include "PybindQueryTypes.h"
#include "PybindReclassify.h"
#include "PybindTypeSet.h"

#include "PybindPointCloud.h"
#include "PybindPointCloudGenerator.h"
#include "PybindStructuredGrid.h"
#include "PybindStructuredGridGenerator.h"

#include "PybindDeleteMesh.h"
#include "PybindElevateMesh.h"
#include "PybindExport.h"
#include "PybindImport.h"
#include "PybindInterpolateOntoMesh.h"
#include "PybindRead.h"
#include "PybindWrite.h"

#include "smtk/mesh/interpolation/PointCloudFromCSV.h"

#include "smtk/operation/XMLOperation.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindMesh, mesh)
{
  mesh.doc() = "<description>";

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::mesh::HandleInterval > smtk_mesh_HandleInterval = pybind11_init_HandleInterval(mesh);
  PySharedPtrClass< smtk::mesh::const_element_iterator > smtk_mesh_const_element_iterator = pybind11_init_const_element_iterator(mesh);
  py::class_< smtk::mesh::HandleRange > smtk_mesh_HandleRange = pybind11_init_HandleRange(mesh);

  PySharedPtrClass< smtk::mesh::Allocator > smtk_mesh_Allocator = pybind11_init_smtk_mesh_Allocator(mesh);
  PySharedPtrClass< smtk::mesh::BufferedCellAllocator > smtk_mesh_BufferedCellAllocator = pybind11_init_smtk_mesh_BufferedCellAllocator(mesh);
  PySharedPtrClass< smtk::mesh::IncrementalAllocator > smtk_mesh_IncrementalAllocator = pybind11_init_smtk_mesh_IncrementalAllocator(mesh);
  pybind11_init_smtk_mesh_DimensionType(mesh);
  pybind11_init_smtk_mesh_FieldType(mesh);
  PySharedPtrClass< smtk::mesh::CellForEach > smtk_mesh_CellForEach = pybind11_init_smtk_mesh_CellForEach(mesh);
  PySharedPtrClass< smtk::mesh::CellSet > smtk_mesh_CellSet = pybind11_init_smtk_mesh_CellSet(mesh);
  PySharedPtrClass< smtk::mesh::Collection > smtk_mesh_Collection = pybind11_init_smtk_mesh_Collection(mesh);
  PySharedPtrClass< smtk::mesh::ConnectivityStorage > smtk_mesh_ConnectivityStorage = pybind11_init_smtk_mesh_ConnectivityStorage(mesh);
  PySharedPtrClass< smtk::mesh::CellField > smtk_mesh_CellField = pybind11_init_smtk_mesh_CellField(mesh);
  PySharedPtrClass< smtk::mesh::CellField > smtk_mesh_PointField = pybind11_init_smtk_mesh_PointField(mesh);
  PySharedPtrClass< smtk::mesh::utility::MeshConstants > smtk_mesh_MeshConstants = pybind11_init_smtk_mesh_MeshConstants(mesh);
  PySharedPtrClass< smtk::mesh::IntegerTag > smtk_mesh_IntegerTag = pybind11_init_smtk_mesh_IntegerTag(mesh);
  PySharedPtrClass< smtk::mesh::Interface > smtk_mesh_Interface = pybind11_init_smtk_mesh_Interface(mesh);
  PySharedPtrClass< smtk::mesh::MeshForEach > smtk_mesh_MeshForEach = pybind11_init_smtk_mesh_MeshForEach(mesh);
  PySharedPtrClass< smtk::mesh::MeshSet > smtk_mesh_MeshSet = pybind11_init_smtk_mesh_MeshSet(mesh);
  PySharedPtrClass< smtk::mesh::OpaqueTag<16> > smtk_mesh_OpaqueTag_16_ = pybind11_init_smtk_mesh_OpaqueTag_16_(mesh);
  PySharedPtrClass< smtk::mesh::PointConnectivity > smtk_mesh_PointConnectivity = pybind11_init_smtk_mesh_PointConnectivity(mesh);
  PySharedPtrClass< smtk::mesh::PointForEach > smtk_mesh_PointForEach = pybind11_init_smtk_mesh_PointForEach(mesh);
  PySharedPtrClass< smtk::mesh::PointLocator > smtk_mesh_PointLocator = pybind11_init_smtk_mesh_PointLocator(mesh);
  PySharedPtrClass< smtk::mesh::PointLocatorImpl > smtk_mesh_PointLocatorImpl = pybind11_init_smtk_mesh_PointLocatorImpl(mesh);
  PySharedPtrClass< smtk::mesh::PointSet > smtk_mesh_PointSet = pybind11_init_smtk_mesh_PointSet(mesh);
  PySharedPtrClass< smtk::mesh::utility::PreAllocatedMeshConstants > smtk_mesh_PreAllocatedMeshConstants = pybind11_init_smtk_mesh_PreAllocatedMeshConstants(mesh);
  PySharedPtrClass< smtk::mesh::utility::PreAllocatedTessellation > smtk_mesh_PreAllocatedTessellation = pybind11_init_smtk_mesh_PreAllocatedTessellation(mesh);
  PySharedPtrClass< smtk::mesh::utility::Tessellation > smtk_mesh_Tessellation = pybind11_init_smtk_mesh_Tessellation(mesh);
  PySharedPtrClass< smtk::mesh::TypeSet > smtk_mesh_TypeSet = pybind11_init_smtk_mesh_TypeSet(mesh);

  pybind11_init_smtk_mesh_rangeElementsBegin(mesh);
  pybind11_init_smtk_mesh_rangeElementsEnd(mesh);
  pybind11_init_smtk_mesh_rangeElement(mesh);
  pybind11_init_smtk_mesh_rangeContains(mesh);
  pybind11_init_smtk_mesh_rangeIndex(mesh);
  pybind11_init_smtk_mesh_rangeIntervalCount(mesh);
  pybind11_init_smtk_mesh_rangesEqual(mesh);
  pybind11_init_smtk_mesh_CellType(mesh);
  pybind11_init_smtk_mesh_ContainmentType(mesh);
  pybind11_init_smtk_mesh_cellTypeSummary(mesh);
  pybind11_init__ZN4smtk4mesh21extractDirichletMeshConstantsERKNS0_7MeshSetERNS0_17PreAllocatedMeshConstantsE(mesh);
  pybind11_init__ZN4smtk4mesh21extractDirichletMeshConstantsERKNS0_7MeshSetERKNS0_8PointSetERNS0_17PreAllocatedMeshConstantsE(mesh);
  pybind11_init__ZN4smtk4mesh18extractDomainMeshConstantsERKNS0_7MeshSetERNS0_17PreAllocatedMeshConstantsE(mesh);
  pybind11_init__ZN4smtk4mesh18extractDomainMeshConstantsERKNS0_7MeshSetERKNS0_8PointSetERNS0_17PreAllocatedMeshConstantsE(mesh);
  pybind11_init__ZN4smtk4mesh19extractNeumannMeshConstantsERKNS0_7MeshSetERNS0_17PreAllocatedMeshConstantsE(mesh);
  pybind11_init__ZN4smtk4mesh19extractNeumannMeshConstantsERKNS0_7MeshSetERKNS0_8PointSetERNS0_17PreAllocatedMeshConstantsE(mesh);
  pybind11_init__ZN4smtk4mesh26extractOrderedTessellationERKNS_5model4EdgeERKNSt3__110shared_ptrINS0_10CollectionEEERNS0_24PreAllocatedTessellationE(mesh);
  pybind11_init__ZN4smtk4mesh26extractOrderedTessellationERKNS_5model4LoopERKNSt3__110shared_ptrINS0_10CollectionEEERNS0_24PreAllocatedTessellationE(mesh);
  pybind11_init__ZN4smtk4mesh26extractOrderedTessellationERKNS_5model4EdgeERKNSt3__110shared_ptrINS0_10CollectionEEERKNS0_8PointSetERNS0_24PreAllocatedTessellationE(mesh);
  pybind11_init__ZN4smtk4mesh26extractOrderedTessellationERKNS_5model4LoopERKNSt3__110shared_ptrINS0_10CollectionEEERKNS0_8PointSetERNS0_24PreAllocatedTessellationE(mesh);
  pybind11_init__ZN4smtk4mesh19extractTessellationERKNS0_7MeshSetERNS0_24PreAllocatedTessellationE(mesh);
  pybind11_init__ZN4smtk4mesh19extractTessellationERKNS0_7CellSetERNS0_24PreAllocatedTessellationE(mesh);
  pybind11_init__ZN4smtk4mesh19extractTessellationERKNS_5model9EntityRefERKNSt3__110shared_ptrINS0_10CollectionEEERNS0_24PreAllocatedTessellationE(mesh);
  pybind11_init__ZN4smtk4mesh19extractTessellationERKNS0_7MeshSetERKNS0_8PointSetERNS0_24PreAllocatedTessellationE(mesh);
  pybind11_init__ZN4smtk4mesh19extractTessellationERKNS0_7CellSetERKNS0_8PointSetERNS0_24PreAllocatedTessellationE(mesh);
  pybind11_init__ZN4smtk4mesh19extractTessellationERNS0_17PointConnectivityERKNS0_8PointSetERNS0_24PreAllocatedTessellationE(mesh);
  pybind11_init__ZN4smtk4mesh19extractTessellationERKNS_5model9EntityRefERKNSt3__110shared_ptrINS0_10CollectionEEERKNS0_8PointSetERNS0_24PreAllocatedTessellationE(mesh);
  pybind11_init_smtk_mesh_metrics(mesh);
  pybind11_init_smtk_mesh_utility_applyScalarCellField(mesh);
  pybind11_init_smtk_mesh_utility_applyScalarPointField(mesh);
  pybind11_init_smtk_mesh_utility_applyVectorCellField(mesh);
  pybind11_init_smtk_mesh_utility_applyVectorPointField(mesh);
  pybind11_init_smtk_mesh_utility_applyWarp(mesh);
  pybind11_init_smtk_mesh_utility_undoWarp(mesh);
  pybind11_init_smtk_mesh_cell_for_each(mesh);
  pybind11_init_smtk_mesh_mesh_for_each(mesh);
  pybind11_init_smtk_mesh_point_for_each(mesh);
  pybind11_init_smtk_mesh_fuse(mesh);
  pybind11_init_smtk_mesh_make_disjoint(mesh);
  pybind11_init_smtk_mesh_merge(mesh);
  pybind11_init_smtk_mesh_cell_point_difference(mesh);
  pybind11_init_smtk_mesh_cell_point_intersect(mesh);
  pybind11_init_smtk_mesh_cell_set_difference(mesh);
  pybind11_init_smtk_mesh_mesh_set_difference(mesh);
  pybind11_init_smtk_mesh_point_set_difference(mesh);
  pybind11_init_smtk_mesh_cell_set_intersect(mesh);
  pybind11_init_smtk_mesh_mesh_set_intersect(mesh);
  pybind11_init_smtk_mesh_point_set_intersect(mesh);
  pybind11_init_smtk_mesh_cell_set_union(mesh);
  pybind11_init_smtk_mesh_mesh_set_union(mesh);
  pybind11_init_smtk_mesh_point_set_union(mesh);
  pybind11_init_smtk_mesh_split(mesh);
  pybind11_init_smtk_mesh_verticesPerCell(mesh);
  PySharedPtrClass< smtk::mesh::Dirichlet > smtk_mesh_Dirichlet = pybind11_init_smtk_mesh_Dirichlet(mesh, smtk_mesh_IntegerTag);
  PySharedPtrClass< smtk::mesh::Domain > smtk_mesh_Domain = pybind11_init_smtk_mesh_Domain(mesh, smtk_mesh_IntegerTag);
  PySharedPtrClass< smtk::mesh::Neumann > smtk_mesh_Neumann = pybind11_init_smtk_mesh_Neumann(mesh, smtk_mesh_IntegerTag);
  PySharedPtrClass< smtk::mesh::UUIDTag > smtk_mesh_UUIDTag = pybind11_init_smtk_mesh_UUIDTag(mesh, smtk_mesh_OpaqueTag_16_);
  PySharedPtrClass< smtk::mesh::Model > smtk_mesh_Model = pybind11_init_smtk_mesh_Model(mesh, smtk_mesh_UUIDTag);
  PySharedPtrClass< smtk::mesh::Component > smtk_mesh_Component = pybind11_init_smtk_mesh_Component(mesh);

  PySharedPtrClass< smtk::mesh::DeleteMesh, smtk::operation::XMLOperation > smtk_mesh_DeleteMesh = pybind11_init_smtk_mesh_DeleteMesh(mesh);
  PySharedPtrClass< smtk::mesh::ElevateMesh, smtk::operation::XMLOperation > smtk_mesh_ElevateMesh = pybind11_init_smtk_mesh_ElevateMesh(mesh);
  PySharedPtrClass< smtk::mesh::Export, smtk::operation::XMLOperation > smtk_mesh_Export = pybind11_init_smtk_mesh_Export(mesh);
  PySharedPtrClass< smtk::mesh::InterpolateOntoMesh, smtk::operation::XMLOperation > smtk_mesh_InterpolateOntoMesh = pybind11_init_smtk_mesh_InterpolateOntoMesh(mesh);
  PySharedPtrClass< smtk::mesh::Import, smtk::operation::XMLOperation > smtk_mesh_Import = pybind11_init_smtk_mesh_Import(mesh);
  PySharedPtrClass< smtk::mesh::Read, smtk::operation::XMLOperation > smtk_mesh_Read = pybind11_init_smtk_mesh_Read(mesh);
  PySharedPtrClass< smtk::mesh::Write, smtk::operation::XMLOperation > smtk_mesh_Write = pybind11_init_smtk_mesh_Write(mesh);

  py::class_< smtk::mesh::PointCloud > smtk_mesh_PointCloud = pybind11_init_smtk_mesh_PointCloud(mesh);
  py::class_< smtk::mesh::StructuredGrid > smtk_mesh_StructuredGrid = pybind11_init_smtk_mesh_StructuredGrid(mesh);
  py::class_< smtk::mesh::PointCloudGenerator > smtk_mesh_PointCloudGenerator = pybind11_init_smtk_mesh_PointCloudGenerator(mesh);
  py::class_< smtk::mesh::StructuredGridGenerator > smtk_mesh_StructuredGridGenerator = pybind11_init_smtk_mesh_StructuredGridGenerator(mesh);

  bool pcRegistered = smtk::mesh::PointCloudFromCSV::registerClass();
  (void)pcRegistered;
}
