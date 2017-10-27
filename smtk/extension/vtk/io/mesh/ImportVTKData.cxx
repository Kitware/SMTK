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

#include "smtk/mesh/CellField.h"
#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/CellTraits.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/ExtractTessellation.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/PointField.h"

#include "vtkAOSDataArrayTemplate.h"
#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkDataSetReader.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkXMLUnstructuredGridWriter.h"

#include "vtksys/SystemTools.hxx"

#include "moab/ReadUtilIface.hpp"

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
      ctype = smtk::mesh::Line;
      break;
    case VTK_TRIANGLE:
      ctype = smtk::mesh::Triangle;
      break;
    case VTK_QUAD:
      ctype = smtk::mesh::Quad;
      break;
    case VTK_POLYGON:
      ctype = smtk::mesh::Polygon;
      break;
    case VTK_TETRA:
      ctype = smtk::mesh::Tetrahedron;
      break;
    case VTK_PYRAMID:
      ctype = smtk::mesh::Pyramid;
      break;
    case VTK_WEDGE:
      ctype = smtk::mesh::Wedge;
      break;
    case VTK_HEXAHEDRON:
      ctype = smtk::mesh::Hexahedron;
      break;
    default:
      ctype = smtk::mesh::CellType_MAX;
      break;
  }
  return ctype;
}

template <typename VTKDataSetType>
smtk::mesh::HandleRange convertVTKDataSet(
  VTKDataSetType* dataset, smtk::mesh::CollectionPtr& collection)
{
  smtk::mesh::InterfacePtr iface = collection->interface();
  smtk::mesh::BufferedCellAllocatorPtr alloc = iface->bufferedCellAllocator();

  smtk::mesh::HandleRange initRange = collection->cells().range();

  if (!alloc->reserveNumberOfCoordinates(dataset->GetPoints()->GetNumberOfPoints()))
  {
    return smtk::mesh::HandleRange();
  }

  //note this could become a performance bottleneck. If that occurs
  //we will need to move to a template dispatch solution to handle floats,
  //doubles, and vtk Mapped Arrays
  double point[3];
  for (vtkIdType i = 0; i < dataset->GetPoints()->GetNumberOfPoints(); ++i)
  {
    dataset->GetPoints()->GetPoint(i, point);
    alloc->setCoordinate(i, point);
  }

  vtkIdType npts, *pts;
  for (vtkIdType i = 0; i < dataset->GetNumberOfCells(); ++i)
  {
    dataset->GetCellPoints(i, npts, pts);
    alloc->addCell(vtkToSMTKCell(dataset->GetCellType(i)), pts, npts);
  }
  if (!alloc->flush())
  {
    return smtk::mesh::HandleRange();
  }

  return subtract(alloc->cells(), initRange);
}

template <typename TReader>
vtkDataSet* readFile(const std::string& fileName)
{
  vtkSmartPointer<TReader> reader = vtkSmartPointer<TReader>::New();
  reader->SetFileName(fileName.c_str());
  reader->Update();
  reader->GetOutput()->Register(reader);
  return vtkDataSet::SafeDownCast(reader->GetOutput());
}

smtk::mesh::HandleRange convertDomain(vtkCellData* cellData, const smtk::mesh::InterfacePtr& iface,
  const smtk::mesh::HandleRange& cells, const std::string& materialPropertyName)
{
  if (cellData == NULL)
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

  typedef smtk::mesh::HandleRange::const_iterator cit;
  vtkIdType index = 0;
  for (cit i = cells.begin(); i != cells.end(); ++i, ++index)
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
}

ImportVTKData::ImportVTKData()
{
}

smtk::mesh::CollectionPtr ImportVTKData::operator()(const std::string& filename,
  smtk::mesh::ManagerPtr& manager, std::string materialPropertyName) const
{
  smtk::mesh::CollectionPtr collection = manager->makeCollection();
  return this->operator()(filename, collection, materialPropertyName) ? collection
                                                                      : smtk::mesh::CollectionPtr();
}

bool ImportVTKData::operator()(const std::string& filename, smtk::mesh::CollectionPtr collection,
  std::string materialPropertyName) const
{
  std::string extension = vtksys::SystemTools::GetFilenameLastExtension(filename.c_str());

  // Dispatch based on the file extension
  vtkDataSet* data;
  if (extension == ".vtu")
  {
    data = readFile<vtkXMLUnstructuredGridReader>(filename);
    return this->operator()(
      vtkUnstructuredGrid::SafeDownCast(data), collection, materialPropertyName);
  }
  else if (extension == ".vtp")
  {
    data = readFile<vtkXMLPolyDataReader>(filename);
    return this->operator()(vtkPolyData::SafeDownCast(data), collection, materialPropertyName);
  }
  else if (extension == ".vtk")
  {
    data = readFile<vtkDataSetReader>(filename);
    if (vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast(data))
    {
      return this->operator()(ugrid, collection, materialPropertyName);
    }
    else if (vtkPolyData* polydata = vtkPolyData::SafeDownCast(data))
    {
      return this->operator()(polydata, collection, materialPropertyName);
    }
  }

  return false;
}

smtk::mesh::MeshSet ImportVTKData::operator()(
  vtkPolyData* polydata, smtk::mesh::CollectionPtr collection) const
{
  //make sure we have a valid poly data
  if (!polydata)
  {
    return smtk::mesh::MeshSet();
  }
  else if (polydata->GetNumberOfPoints() == 0 || polydata->GetNumberOfCells() == 0)
  {
    //early terminate if the polydata is empty.
    return smtk::mesh::MeshSet();
  }

  if (polydata->NeedToBuildCells())
  {
    polydata->BuildCells();
  }
  smtk::mesh::HandleRange cells = convertVTKDataSet(polydata, collection);

  smtk::mesh::MeshSet meshset = collection->createMesh(smtk::mesh::CellSet(collection, cells));

  collection->interface()->setModifiedState(false);
  return meshset;
}

bool ImportVTKData::operator()(vtkPolyData* polydata, smtk::mesh::CollectionPtr collection,
  std::string materialPropertyName) const
{
  //make sure we have valid data
  if (!polydata)
  {
    return false;
  }
  else if (polydata->GetNumberOfPoints() == 0 || polydata->GetNumberOfCells() == 0)
  {
    //early terminate if the polydata is empty.
    return false;
  }

  smtk::mesh::InterfacePtr iface = collection->interface();
  smtk::mesh::BufferedCellAllocatorPtr alloc = iface->bufferedCellAllocator();

  if (polydata->NeedToBuildCells())
  {
    polydata->BuildCells();
  }
  smtk::mesh::HandleRange cells = convertVTKDataSet(polydata, collection);

  smtk::mesh::MeshSet mesh;

  if (materialPropertyName.empty())
  { //if we don't have a material we create a single mesh
    smtk::mesh::Handle vtkMeshHandle;
    bool created = iface->createMesh(cells, vtkMeshHandle);
    if (created)
    {
      smtk::mesh::HandleRange entities;
      entities.insert(vtkMeshHandle);
      mesh = smtk::mesh::MeshSet(collection->shared_from_this(), iface->getRoot(), entities);
    }
  }
  else
  { //make multiple meshes each one assigned a material value
    smtk::mesh::HandleRange entities =
      convertDomain(polydata->GetCellData(), iface, cells, materialPropertyName);
    mesh = smtk::mesh::MeshSet(collection->shared_from_this(), iface->getRoot(), entities);
  }

  // Now that we have a valid meshset, we add double-valued vtk cell & point data to it.
  if (!mesh.is_empty())
  {
    for (vtkIdType i = 0; i < polydata->GetCellData()->GetNumberOfArrays(); i++)
    {
      vtkDoubleArray* array = vtkDoubleArray::SafeDownCast(polydata->GetCellData()->GetArray(i));
      if (array != nullptr)
      {
        mesh.createCellField(array->GetName(), array->GetNumberOfComponents(),
          static_cast<const double*>(array->GetVoidPointer(0)));
      }
    }

    for (vtkIdType i = 0; i < polydata->GetPointData()->GetNumberOfArrays(); i++)
    {
      vtkDoubleArray* array = vtkDoubleArray::SafeDownCast(polydata->GetPointData()->GetArray(i));
      if (array != nullptr)
      {
        mesh.createPointField(array->GetName(), array->GetNumberOfComponents(),
          static_cast<const double*>(array->GetVoidPointer(0)));
      }
    }
  }
  iface->setModifiedState(false);
  return !mesh.is_empty();
}

smtk::mesh::CollectionPtr ImportVTKData::operator()(
  vtkPolyData* polydata, smtk::mesh::ManagerPtr& manager, std::string materialPropertyName) const
{
  smtk::mesh::CollectionPtr c = manager->makeCollection();
  return this->operator()(polydata, c, materialPropertyName) ? c : smtk::mesh::CollectionPtr();
}

bool ImportVTKData::operator()(vtkUnstructuredGrid* ugrid, smtk::mesh::CollectionPtr collection,
  std::string materialPropertyName) const
{
  //make sure we have valid data
  if (!ugrid)
  {
    return false;
  }
  else if (ugrid->GetNumberOfPoints() == 0 || ugrid->GetNumberOfCells() == 0)
  {
    //early terminate if the ugrid is empty.
    return false;
  }

  smtk::mesh::InterfacePtr iface = collection->interface();
  smtk::mesh::BufferedCellAllocatorPtr alloc = iface->bufferedCellAllocator();

  smtk::mesh::HandleRange cells = convertVTKDataSet(ugrid, collection);

  smtk::mesh::MeshSet mesh;

  if (materialPropertyName.empty())
  { //if we don't have a material we create a single mesh
    smtk::mesh::Handle vtkMeshHandle;
    bool created = iface->createMesh(cells, vtkMeshHandle);
    if (created)
    {
      smtk::mesh::HandleRange entities;
      entities.insert(vtkMeshHandle);
      mesh = smtk::mesh::MeshSet(collection->shared_from_this(), iface->getRoot(), entities);
    }
  }
  else
  { //make multiple meshes each one assigned a material value
    smtk::mesh::HandleRange entities =
      convertDomain(ugrid->GetCellData(), iface, cells, materialPropertyName);
    mesh = smtk::mesh::MeshSet(collection->shared_from_this(), iface->getRoot(), entities);
  }

  // Now that we have a valid meshset, we add double-valued vtk cell & point data to it.
  if (!mesh.is_empty())
  {
    for (vtkIdType i = 0; i < ugrid->GetCellData()->GetNumberOfArrays(); i++)
    {
      vtkDoubleArray* array = vtkDoubleArray::SafeDownCast(ugrid->GetCellData()->GetArray(i));
      if (array != nullptr)
      {
        mesh.createCellField(array->GetName(), array->GetNumberOfComponents(),
          static_cast<const double*>(array->GetVoidPointer(0)));
      }
    }

    for (vtkIdType i = 0; i < ugrid->GetPointData()->GetNumberOfArrays(); i++)
    {
      vtkDoubleArray* array = vtkDoubleArray::SafeDownCast(ugrid->GetPointData()->GetArray(i));
      if (array != nullptr)
      {
        mesh.createPointField(array->GetName(), array->GetNumberOfComponents(),
          static_cast<const double*>(array->GetVoidPointer(0)));
      }
    }
  }
  iface->setModifiedState(false);
  return !mesh.is_empty();
}

smtk::mesh::CollectionPtr ImportVTKData::operator()(vtkUnstructuredGrid* ugrid,
  smtk::mesh::ManagerPtr& manager, std::string materialPropertyName) const
{
  smtk::mesh::CollectionPtr c = manager->makeCollection();
  return this->operator()(ugrid, c, materialPropertyName) ? c : smtk::mesh::CollectionPtr();
}
}
}
}
}
}
