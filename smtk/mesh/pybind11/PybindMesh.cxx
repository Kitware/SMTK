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

#include "PybindCellField.h"
#include "PybindCellSet.h"
#include "PybindCellTypes.h"
#include "PybindCollection.h"
#include "PybindContainsFunctors.h"
#include "PybindDimensionTypes.h"
#include "PybindDisplace.h"
#include "PybindExtractMeshConstants.h"
#include "PybindExtractTessellation.h"
#include "PybindForEachTypes.h"
#include "PybindHandle.h"
#include "PybindHandleRange.h"
#include "PybindInterface.h"
#include "PybindManager.h"
#include "PybindMeshSet.h"
#include "PybindPointField.h"
#include "PybindPointConnectivity.h"
#include "PybindPointLocator.h"
#include "PybindPointSet.h"
#include "PybindQueryTypes.h"
#include "PybindReclassify.h"
#include "PybindTypeSet.h"

#include "PybindDeleteMesh.h"
#include "PybindExportMesh.h"
#include "PybindInterpolateMesh.h"
#include "PybindWriteMesh.h"

#include "smtk/model/Operator.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_PLUGIN(_smtkPybindMesh)
{
  py::module mesh("_smtkPybindMesh", "<description>");
  py::module moab = mesh.def_submodule("moab", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  pybind11_init_moab_EntityType(moab);
  PySharedPtrClass< moab::Range > smtk_mesh_moab_Range = pybind11_init_moab_Range(moab);
  pybind11_init_moab_range_base_iter(moab);
  pybind11_init_moab_range_inserter(moab);
  pybind11_init_moab_range_iter_tag(moab);
  pybind11_init_std_bidirectional_iterator_tag(moab);
  PySharedPtrClass< smtk::mesh::Allocator > smtk_mesh_Allocator = pybind11_init_smtk_mesh_Allocator(mesh);
  pybind11_init_smtk_mesh_DimensionType(mesh);
  PySharedPtrClass< smtk::mesh::CellForEach > smtk_mesh_CellForEach = pybind11_init_smtk_mesh_CellForEach(mesh);
  PySharedPtrClass< smtk::mesh::CellSet > smtk_mesh_CellSet = pybind11_init_smtk_mesh_CellSet(mesh);
  PySharedPtrClass< smtk::mesh::Collection > smtk_mesh_Collection = pybind11_init_smtk_mesh_Collection(mesh);
  PySharedPtrClass< smtk::mesh::ConnectivityStorage > smtk_mesh_ConnectivityStorage = pybind11_init_smtk_mesh_ConnectivityStorage(mesh);
  PySharedPtrClass< smtk::mesh::ContainsFunctor > smtk_mesh_ContainsFunctor = pybind11_init_smtk_mesh_ContainsFunctor(mesh);
  PySharedPtrClass< smtk::mesh::CellField > smtk_mesh_CellField = pybind11_init_smtk_mesh_CellField(mesh);
  PySharedPtrClass< smtk::mesh::CellField > smtk_mesh_PointField = pybind11_init_smtk_mesh_PointField(mesh);
  PySharedPtrClass< smtk::mesh::ElevationControls > smtk_mesh_ElevationControls = pybind11_init_smtk_mesh_ElevationControls(mesh);
  PySharedPtrClass< smtk::mesh::MeshConstants > smtk_mesh_MeshConstants = pybind11_init_smtk_mesh_MeshConstants(mesh);
  PySharedPtrClass< smtk::mesh::IntegerTag > smtk_mesh_IntegerTag = pybind11_init_smtk_mesh_IntegerTag(mesh);
  PySharedPtrClass< smtk::mesh::Interface > smtk_mesh_Interface = pybind11_init_smtk_mesh_Interface(mesh);
  PySharedPtrClass< smtk::mesh::Manager > smtk_mesh_Manager = pybind11_init_smtk_mesh_Manager(mesh);
  PySharedPtrClass< smtk::mesh::MeshForEach > smtk_mesh_MeshForEach = pybind11_init_smtk_mesh_MeshForEach(mesh);
  PySharedPtrClass< smtk::mesh::MeshSet > smtk_mesh_MeshSet = pybind11_init_smtk_mesh_MeshSet(mesh);
  PySharedPtrClass< smtk::mesh::OpaqueTag<16> > smtk_mesh_OpaqueTag_16_ = pybind11_init_smtk_mesh_OpaqueTag_16_(mesh);
  PySharedPtrClass< smtk::mesh::PointConnectivity > smtk_mesh_PointConnectivity = pybind11_init_smtk_mesh_PointConnectivity(mesh);
  PySharedPtrClass< smtk::mesh::PointForEach > smtk_mesh_PointForEach = pybind11_init_smtk_mesh_PointForEach(mesh);
  PySharedPtrClass< smtk::mesh::PointLocator > smtk_mesh_PointLocator = pybind11_init_smtk_mesh_PointLocator(mesh);
  PySharedPtrClass< smtk::mesh::PointLocatorImpl > smtk_mesh_PointLocatorImpl = pybind11_init_smtk_mesh_PointLocatorImpl(mesh);
  PySharedPtrClass< smtk::mesh::PointSet > smtk_mesh_PointSet = pybind11_init_smtk_mesh_PointSet(mesh);
  PySharedPtrClass< smtk::mesh::PreAllocatedMeshConstants > smtk_mesh_PreAllocatedMeshConstants = pybind11_init_smtk_mesh_PreAllocatedMeshConstants(mesh);
  PySharedPtrClass< smtk::mesh::PreAllocatedTessellation > smtk_mesh_PreAllocatedTessellation = pybind11_init_smtk_mesh_PreAllocatedTessellation(mesh);
  PySharedPtrClass< smtk::mesh::Tessellation > smtk_mesh_Tessellation = pybind11_init_smtk_mesh_Tessellation(mesh);
  PySharedPtrClass< smtk::mesh::TypeSet > smtk_mesh_TypeSet = pybind11_init_smtk_mesh_TypeSet(mesh);
  pybind11_init_smtk_mesh_CellType(mesh);
  pybind11_init_smtk_mesh_ContainmentType(mesh);
  pybind11_init_smtk_mesh_cellTypeSummary(mesh);
  pybind11_init__ZN4smtk4mesh8displaceERKNS0_8PointSetERKNS0_7MeshSetEd(mesh);
  pybind11_init__ZN4smtk4mesh8displaceERKNS0_8PointSetES3_d(mesh);
  pybind11_init__ZN4smtk4mesh7elevateERKNSt3__16vectorIdNS1_9allocatorIdEEEERKNS0_7MeshSetEdNS0_17ElevationControlsE(mesh);
  pybind11_init__ZN4smtk4mesh7elevateERKNSt3__16vectorIdNS1_9allocatorIdEEEERKNS0_8PointSetEdNS0_17ElevationControlsE(mesh);
  pybind11_init__ZN4smtk4mesh7elevateEPKdmRKNS0_7MeshSetEdNS0_17ElevationControlsE(mesh);
  pybind11_init__ZN4smtk4mesh7elevateEPKfmRKNS0_7MeshSetEdNS0_17ElevationControlsE(mesh);
  pybind11_init__ZN4smtk4mesh7elevateEPKdmRKNS0_8PointSetEdNS0_17ElevationControlsE(mesh);
  pybind11_init__ZN4smtk4mesh7elevateEPKfmRKNS0_8PointSetEdNS0_17ElevationControlsE(mesh);
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
  pybind11_init_smtk_mesh_cell_for_each(mesh);
  pybind11_init_smtk_mesh_mesh_for_each(mesh);
  pybind11_init_smtk_mesh_point_for_each(mesh);
  pybind11_init_smtk_mesh_from_json(mesh);
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
  pybind11_init_smtk_mesh_to_json(mesh);
  pybind11_init_smtk_mesh_verticesPerCell(mesh);
  PySharedPtrClass< smtk::mesh::Dirichlet > smtk_mesh_Dirichlet = pybind11_init_smtk_mesh_Dirichlet(mesh, smtk_mesh_IntegerTag);
  PySharedPtrClass< smtk::mesh::Domain > smtk_mesh_Domain = pybind11_init_smtk_mesh_Domain(mesh, smtk_mesh_IntegerTag);
  PySharedPtrClass< smtk::mesh::FullyContainedFunctor > smtk_mesh_FullyContainedFunctor = pybind11_init_smtk_mesh_FullyContainedFunctor(mesh, smtk_mesh_ContainsFunctor);
  PySharedPtrClass< smtk::mesh::Neumann > smtk_mesh_Neumann = pybind11_init_smtk_mesh_Neumann(mesh, smtk_mesh_IntegerTag);
  PySharedPtrClass< smtk::mesh::PartiallyContainedFunctor > smtk_mesh_PartiallyContainedFunctor = pybind11_init_smtk_mesh_PartiallyContainedFunctor(mesh, smtk_mesh_ContainsFunctor);
  PySharedPtrClass< smtk::mesh::UUIDTag > smtk_mesh_UUIDTag = pybind11_init_smtk_mesh_UUIDTag(mesh, smtk_mesh_OpaqueTag_16_);
  PySharedPtrClass< smtk::mesh::Model > smtk_mesh_Model = pybind11_init_smtk_mesh_Model(mesh, smtk_mesh_UUIDTag);

  PySharedPtrClass< smtk::mesh::DeleteMesh, smtk::model::Operator > smtk_mesh_DeleteMesh = pybind11_init_smtk_mesh_DeleteMesh(mesh);
  PySharedPtrClass< smtk::mesh::ExportMesh, smtk::model::Operator > smtk_mesh_ExportMesh = pybind11_init_smtk_mesh_ExportMesh(mesh);
  PySharedPtrClass< smtk::mesh::InterpolateMesh, smtk::model::Operator > smtk_mesh_InterpolateMesh = pybind11_init_smtk_mesh_InterpolateMesh(mesh);
  PySharedPtrClass< smtk::mesh::WriteMesh, smtk::model::Operator > smtk_mesh_WriteMesh = pybind11_init_smtk_mesh_WriteMesh(mesh);

  return mesh.ptr();
}
