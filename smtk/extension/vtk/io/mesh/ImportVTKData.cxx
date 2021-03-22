//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#include "smtk/extension/vtk/io/mesh/ImportVTKData.h"

#include "smtk/mesh/core/CellField.h"
#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/CellTraits.h"
#include "smtk/mesh/core/FieldTypes.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/PointField.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/utility/ExtractTessellation.h"

#include "vtkAOSDataArrayTemplate.h"
#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkImageData.h"
#include "vtkIntArray.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkTriangleFilter.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnstructuredGrid.h"

#include "vtksys/SystemTools.hxx"

#include "moab/ReadUtilIface.hpp"

#include "smtk/extension/vtk/io/ImportAsVTKData.h"

#include "smtk/mesh/moab/CellTypeToType.h"
#include "smtk/mesh/moab/Interface.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace io
{
namespace mesh
{
namespace
{

smtk::mesh::CellType vtkToSMTKCell(int t)
{
  smtk::mesh::CellType ctype = smtk::mesh::CellType_MAX;
  switch (t)
  {
    case VTK_VERTEX:
      ctype = smtk::mesh::Vertex;
      break;
    case VTK_LINE:
    case VTK_QUADRATIC_EDGE:
    case VTK_CUBIC_LINE:
    case VTK_PARAMETRIC_CURVE:
    case VTK_HIGHER_ORDER_EDGE:
    case VTK_LAGRANGE_CURVE:
      ctype = smtk::mesh::Line;
      break;
    case VTK_TRIANGLE:
    case VTK_QUADRATIC_TRIANGLE:
    case VTK_BIQUADRATIC_TRIANGLE:
    case VTK_PARAMETRIC_TRI_SURFACE:
    case VTK_HIGHER_ORDER_TRIANGLE:
    case VTK_LAGRANGE_TRIANGLE:
      ctype = smtk::mesh::Triangle;
      break;
    case VTK_QUAD:
    case VTK_PIXEL:
    case VTK_QUADRATIC_QUAD:
    case VTK_BIQUADRATIC_QUAD:
    case VTK_QUADRATIC_LINEAR_QUAD:
    case VTK_PARAMETRIC_QUAD_SURFACE:
    case VTK_HIGHER_ORDER_QUAD:
    case VTK_LAGRANGE_QUADRILATERAL:
      ctype = smtk::mesh::Quad;
      break;
    case VTK_POLYGON:
    case VTK_QUADRATIC_POLYGON:
    case VTK_HIGHER_ORDER_POLYGON:
      ctype = smtk::mesh::Polygon;
      break;
    case VTK_TETRA:
    case VTK_QUADRATIC_TETRA:
    case VTK_PARAMETRIC_TETRA_REGION:
    case VTK_HIGHER_ORDER_TETRAHEDRON:
    case VTK_LAGRANGE_TETRAHEDRON:
      ctype = smtk::mesh::Tetrahedron;
      break;
    case VTK_PYRAMID:
    case VTK_QUADRATIC_PYRAMID:
    case VTK_HIGHER_ORDER_PYRAMID:
    case VTK_LAGRANGE_PYRAMID:
      ctype = smtk::mesh::Pyramid;
      break;
    case VTK_WEDGE:
    case VTK_QUADRATIC_WEDGE:
    case VTK_QUADRATIC_LINEAR_WEDGE:
    case VTK_BIQUADRATIC_QUADRATIC_WEDGE:
    case VTK_HIGHER_ORDER_WEDGE:
    case VTK_LAGRANGE_WEDGE:
      ctype = smtk::mesh::Wedge;
      break;
    case VTK_HEXAHEDRON:
    case VTK_VOXEL:
    case VTK_QUADRATIC_HEXAHEDRON:
    case VTK_TRIQUADRATIC_HEXAHEDRON:
    case VTK_BIQUADRATIC_QUADRATIC_HEXAHEDRON:
    case VTK_PARAMETRIC_HEX_REGION:
    case VTK_HIGHER_ORDER_HEXAHEDRON:
    case VTK_LAGRANGE_HEXAHEDRON:
      ctype = smtk::mesh::Hexahedron;
      break;
    default:
      ctype = smtk::mesh::CellType_MAX;
      break;
  }
  return ctype;
}

smtk::mesh::HandleRange convertVTKDataSet(vtkDataSet* dataset, smtk::mesh::ResourcePtr& resource)
{
  smtk::mesh::InterfacePtr iface = resource->interface();
  smtk::mesh::BufferedCellAllocatorPtr alloc = iface->bufferedCellAllocator();

  smtk::mesh::HandleRange initRange = resource->cells().range();

  if (!alloc->reserveNumberOfCoordinates(dataset->GetNumberOfPoints()))
  {
    return smtk::mesh::HandleRange();
  }

  //note this could become a performance bottleneck. If that occurs
  //we will need to move to a template dispatch solution to handle floats,
  //doubles, and vtk Mapped Arrays
  double point[3];
  for (vtkIdType i = 0; i < dataset->GetNumberOfPoints(); ++i)
  {
    dataset->GetPoint(i, point);
    alloc->setCoordinate(i, point);
  }

  vtkNew<vtkIdList> pts;
  for (vtkIdType i = 0; i < dataset->GetNumberOfCells(); ++i)
  {
    dataset->GetCellPoints(i, pts);
    vtkIdType* ptPtr = pts->GetPointer(0);
    auto cellType = dataset->GetCellType(i);
    {
      vtkIdType tmp;
      if (cellType == VTK_PIXEL || cellType == VTK_VOXEL)
      {
        tmp = ptPtr[2];
        ptPtr[2] = ptPtr[3];
        ptPtr[3] = tmp;
        if (cellType == VTK_VOXEL)
        {
          tmp = ptPtr[6];
          ptPtr[6] = ptPtr[7];
          ptPtr[7] = tmp;
        }
      }
    }
    alloc->addCell(vtkToSMTKCell(cellType), ptPtr, pts->GetNumberOfIds());
  }
  if (!alloc->flush())
  {
    return smtk::mesh::HandleRange();
  }

  return alloc->cells() - initRange;
}

smtk::mesh::HandleRange convertDomain(
  vtkCellData* cellData,
  const smtk::mesh::InterfacePtr& iface,
  const smtk::mesh::HandleRange& cells,
  const std::string& materialPropertyName)
{
  if (cellData == nullptr)
  {
    //we have no information
    return smtk::mesh::HandleRange();
  }

  vtkDataArray* materialData = cellData->GetArray(materialPropertyName.c_str());
  if (!materialData || materialData->GetNumberOfComponents() != 1)
  { //needs to be a scalar array
    return smtk::mesh::HandleRange();
  }

  if (materialData->GetNumberOfTuples() != static_cast<int>(cells.size()))
  { //we currently don't support applying material when
    //we only loaded in some of the cells
    return smtk::mesh::HandleRange();
  }

  std::map<int, smtk::mesh::HandleRange> meshes;
  //move each cell from the entire pool, into a range
  //that represents that material mesh. This is slowish.

  vtkIdType index = 0;
  for (auto i = smtk::mesh::rangeElementsBegin(cells); i != smtk::mesh::rangeElementsEnd(cells);
       ++i, ++index)
  {
    const int currentMaterial = static_cast<int>(materialData->GetTuple1(index));
    meshes[currentMaterial].insert(*i);
  }

  smtk::mesh::HandleRange meshHandles;
  typedef std::map<int, smtk::mesh::HandleRange>::const_iterator map_cit;
  for (map_cit i = meshes.begin(); i != meshes.end(); ++i)
  {
    smtk::mesh::Handle meshId;
    const bool created = iface->createMesh(i->second, meshId);
    if (created)
    {
      smtk::mesh::HandleRange meshHandlesForDomain;
      meshHandlesForDomain.insert(meshId);
      //assign a material id to the mesh
      iface->setDomain(meshHandlesForDomain, smtk::mesh::Domain(i->first));
      meshHandles.insert(meshId);
    }
  }

  return meshHandles;
}
} // namespace

ImportVTKData::ImportVTKData() = default;

smtk::mesh::ResourcePtr ImportVTKData::operator()(
  const std::string& filename,
  const smtk::mesh::InterfacePtr& interface,
  std::string materialPropertyName) const
{
  smtk::mesh::ResourcePtr resource = smtk::mesh::Resource::create(interface);
  return this->operator()(filename, resource, materialPropertyName) ? resource
                                                                    : smtk::mesh::ResourcePtr();
}

bool ImportVTKData::operator()(
  const std::string& filename,
  smtk::mesh::ResourcePtr resource,
  std::string materialPropertyName) const
{
  ImportAsVTKData importAsVTKData;
  auto data = importAsVTKData(filename);
  if (auto* ugrid = vtkUnstructuredGrid::SafeDownCast(data.GetPointer()))
  {
    return this->operator()(ugrid, resource, materialPropertyName);
  }
  else if (auto* poly = vtkPolyData::SafeDownCast(data.GetPointer()))
  {
    // vtkPolyData can hold polylines, triangle strips, polygons and other
    // hard-to-digest cells. These cells can be deconstructed into SMTK-friendly
    // cells via the vtkTriangleFilter.
    vtkNew<vtkTriangleFilter> triangleFilter;
    triangleFilter->SetInputData(poly);
    triangleFilter->Update();
    return this->operator()(triangleFilter->GetOutput(), resource, materialPropertyName);
  }
  else
  {
    return false;
  }
}

smtk::mesh::MeshSet ImportVTKData::operator()(vtkDataSet* dataset, smtk::mesh::ResourcePtr resource)
  const
{
  //make sure we have a valid dataset
  if (!dataset)
  {
    return smtk::mesh::MeshSet();
  }
  else if (dataset->GetNumberOfPoints() == 0 || dataset->GetNumberOfCells() == 0)
  {
    //early terminate if the dataset is empty.
    return smtk::mesh::MeshSet();
  }

  vtkPolyData* polydata = vtkPolyData::SafeDownCast(dataset);
  if (polydata && polydata->NeedToBuildCells())
  {
    polydata->BuildCells();
  }
  smtk::mesh::HandleRange cells = convertVTKDataSet(dataset, resource);

  smtk::mesh::MeshSet meshset = resource->createMesh(smtk::mesh::CellSet(resource, cells));

  resource->interface()->setModifiedState(false);
  return meshset;
}

bool ImportVTKData::operator()(
  vtkDataSet* dataset,
  smtk::mesh::ResourcePtr resource,
  std::string materialPropertyName) const
{
  //make sure we have valid data
  if (!dataset)
  {
    return false;
  }
  else if (dataset->GetNumberOfPoints() == 0 || dataset->GetNumberOfCells() == 0)
  {
    //early terminate if the dataset is empty.
    return false;
  }

  smtk::mesh::InterfacePtr iface = resource->interface();
  smtk::mesh::BufferedCellAllocatorPtr alloc = iface->bufferedCellAllocator();

  vtkPolyData* polydata = vtkPolyData::SafeDownCast(dataset);
  if (polydata && polydata->NeedToBuildCells())
  {
    polydata->BuildCells();
  }
  smtk::mesh::HandleRange cells = convertVTKDataSet(dataset, resource);

  smtk::mesh::MeshSet mesh;

  if (materialPropertyName.empty())
  { //if we don't have a material we create a single mesh
    smtk::mesh::Handle vtkMeshHandle;
    bool created = iface->createMesh(cells, vtkMeshHandle);
    if (created)
    {
      smtk::mesh::HandleRange entities;
      entities.insert(vtkMeshHandle);
      mesh = smtk::mesh::MeshSet(resource->shared_from_this(), iface->getRoot(), entities);
    }
  }
  else
  { //make multiple meshes each one assigned a material value
    smtk::mesh::HandleRange entities =
      convertDomain(dataset->GetCellData(), iface, cells, materialPropertyName);
    mesh = smtk::mesh::MeshSet(resource->shared_from_this(), iface->getRoot(), entities);
  }

  // Now that we have a valid meshset, we add vtk cell & point data to it.
  if (!mesh.is_empty())
  {
    for (vtkIdType i = 0; i < dataset->GetCellData()->GetNumberOfArrays(); i++)
    {
      {
        vtkDoubleArray* array = vtkDoubleArray::SafeDownCast(dataset->GetCellData()->GetArray(i));
        if (array != nullptr)
        {
          mesh.createCellField(
            array->GetName(),
            array->GetNumberOfComponents(),
            smtk::mesh::FieldType::Double,
            static_cast<const void*>(array->GetVoidPointer(0)));
        }
      }
      {
        vtkIntArray* array = vtkIntArray::SafeDownCast(dataset->GetCellData()->GetArray(i));
        if (array != nullptr)
        {
          mesh.createCellField(
            array->GetName(),
            array->GetNumberOfComponents(),
            smtk::mesh::FieldType::Integer,
            static_cast<const void*>(array->GetVoidPointer(0)));
        }
      }
    }

    for (vtkIdType i = 0; i < dataset->GetPointData()->GetNumberOfArrays(); i++)
    {
      {
        vtkDoubleArray* array = vtkDoubleArray::SafeDownCast(dataset->GetPointData()->GetArray(i));
        if (array != nullptr)
        {
          mesh.createPointField(
            array->GetName(),
            array->GetNumberOfComponents(),
            smtk::mesh::FieldType::Double,
            static_cast<const void*>(array->GetVoidPointer(0)));
        }
      }
      {
        vtkIntArray* array = vtkIntArray::SafeDownCast(dataset->GetPointData()->GetArray(i));
        if (array != nullptr)
        {
          mesh.createPointField(
            array->GetName(),
            array->GetNumberOfComponents(),
            smtk::mesh::FieldType::Integer,
            static_cast<const void*>(array->GetVoidPointer(0)));
        }
      }
    }
  }
  iface->setModifiedState(false);
  return !mesh.is_empty();
}

smtk::mesh::ResourcePtr ImportVTKData::operator()(
  vtkDataSet* dataset,
  const smtk::mesh::InterfacePtr& interface,
  std::string materialPropertyName) const
{
  smtk::mesh::ResourcePtr c = smtk::mesh::Resource::create(interface);
  return this->operator()(dataset, c, materialPropertyName) ? c : smtk::mesh::ResourcePtr();
}
} // namespace mesh
} // namespace io
} // namespace vtk
} // namespace extension
} // namespace smtk
