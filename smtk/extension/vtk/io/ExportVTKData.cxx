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

#include "smtk/extension/vtk/io/ExportVTKData.h"

#include "smtk/mesh/CellField.h"
#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/CellTraits.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/ExtractMeshConstants.h"
#include "smtk/mesh/ExtractTessellation.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/PointField.h"

#include "vtkAOSDataArrayTemplate.h"
#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
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

ExportVTKData::ExportVTKData()
{
}

bool ExportVTKData::operator()(const std::string& filename, smtk::mesh::CollectionPtr collection,
  std::string domainPropertyName) const
{
  std::string extension = vtksys::SystemTools::GetFilenameLastExtension(filename.c_str());

  // Dispatch based on the file extension
  if (extension == ".vtu")
  {
    vtkSmartPointer<vtkUnstructuredGrid> ug = vtkSmartPointer<vtkUnstructuredGrid>::New();
    this->operator()(collection->meshes(), ug, domainPropertyName);
    vtkNew<vtkXMLUnstructuredGridWriter> writer;
    writer->SetFileName(filename.c_str());
    writer->SetInputData(ug);
    writer->Write();
    return true;
  }
  else if (extension == ".vtp")
  {
    vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
    this->operator()(collection->meshes(), pd, domainPropertyName);
    vtkNew<vtkXMLPolyDataWriter> writer;
    writer->SetFileName(filename.c_str());
    writer->SetInputData(pd);
    writer->Write();
    return true;
  }

  return false;
}

namespace
{

// functions to shunt past data transfer if input and output types match
void constructNewArrayIfNecessary(vtkIdType*&, vtkIdType*&, std::int64_t)
{
}
void transferDataIfNecessary(vtkIdType*& in, vtkIdType*& out, std::int64_t)
{
  out = in;
}
void deleteOldArrayIfNecessary(vtkIdType*&, vtkIdType*&)
{
}

// functions for allocation, transfer and deallocation when there is a type
// mismatch
template <typename T>
void constructNewArrayIfNecessary(T*&, vtkIdType*& out, std::int64_t len)
{
  out = new vtkIdType[len];
}

template <typename T>
void transferDataIfNecessary(T*& in, vtkIdType*& out, std::int64_t len)
{
  for (std::int64_t i = 0; i < len; i++)
  {
    out[i] = in[i];
  }
}

template <typename T>
void deleteOldArrayIfNecessary(T*& in, vtkIdType*&)
{
  delete[] in;
}
}

void ExportVTKData::operator()(
  const smtk::mesh::MeshSet& meshset, vtkPolyData* pd, std::string domainPropertyName) const
{
  // We are only exporting the highest dimension cellset starting with 2
  int dimension = 2;
  smtk::mesh::TypeSet types = meshset.types();
  while (dimension >= 0 && !types.hasDimension(static_cast<smtk::mesh::DimensionType>(dimension)))
  {
    --dimension;
  }

  if (dimension < 0)
  {
    // We have been passed a meshset with no elements of dimension 2 or lower.
    return;
  }

  smtk::mesh::CellSet cellset = meshset.cells(static_cast<smtk::mesh::DimensionType>(dimension));

  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //determine the allocation lengths
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(
    cellset, connectivityLength, numberOfCells, numberOfPoints);

  // add the number of cells to the connectivity length to get the length of
  // VTK-style connectivity
  connectivityLength += numberOfCells;

  //create raw data buffers to hold our data
  double* pointsData = new double[3 * numberOfPoints];
  unsigned char* cellTypesData = new unsigned char[numberOfCells];
  std::int64_t* cellLocationsData_ = new std::int64_t[numberOfCells];
  std::int64_t* connectivityData_ = new std::int64_t[connectivityLength];

  //extract tessellation information
  smtk::mesh::PreAllocatedTessellation tess(
    connectivityData_, cellLocationsData_, cellTypesData, pointsData);
  smtk::mesh::extractTessellation(cellset, tess);

  vtkIdType* cellLocationsData;
  {
    constructNewArrayIfNecessary(cellLocationsData_, cellLocationsData, numberOfCells);
    transferDataIfNecessary(cellLocationsData_, cellLocationsData, numberOfCells);
    deleteOldArrayIfNecessary(cellLocationsData_, cellLocationsData);
  }

  vtkIdType* connectivityData;
  {
    constructNewArrayIfNecessary(connectivityData_, connectivityData, connectivityLength);
    transferDataIfNecessary(connectivityData_, connectivityData, connectivityLength);
    deleteOldArrayIfNecessary(connectivityData_, connectivityData);
  }

  // create vtk data arrays to hold our data
  vtkNew<vtkDoubleArray> pointsArray;
  vtkNew<vtkUnsignedCharArray> cellTypes;
  vtkNew<vtkIdTypeArray> cellLocations;
  vtkNew<vtkIdTypeArray> connectivity;

  // transfer ownership of our raw data arrays to the vtk data arrays
  pointsArray->SetNumberOfComponents(3);
  pointsArray->SetArray(
    pointsData, 3 * numberOfPoints, false, vtkDoubleArray::VTK_DATA_ARRAY_DELETE);
  cellTypes->SetArray(
    cellTypesData, numberOfCells, false, vtkUnsignedCharArray::VTK_DATA_ARRAY_DELETE);
  cellLocations->SetArray(
    cellLocationsData, numberOfCells, false, vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);
  connectivity->SetArray(
    connectivityData, connectivityLength, false, vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);

  vtkNew<vtkPoints> points;
  points->SetData(pointsArray.GetPointer());
  pd->SetPoints(points.GetPointer());

  vtkNew<vtkCellArray> cells;
  cells->SetCells(numberOfCells, connectivity.GetPointer());

  if (dimension == 2)
  {
    pd->SetPolys(cells.GetPointer());
  }
  else if (dimension == 1)
  {
    pd->SetLines(cells.GetPointer());
  }
  else if (dimension == 0)
  {
    pd->SetVerts(cells.GetPointer());
  }

  if (!domainPropertyName.empty())
  {
    std::int64_t* cellData_ = new std::int64_t[numberOfCells];
    std::int64_t* pointData_ = new std::int64_t[numberOfPoints];

    //extract mesh constant information
    smtk::mesh::PreAllocatedMeshConstants meshConstants(cellData_, pointData_);
    smtk::mesh::extractDomainMeshConstants(meshset, meshConstants);

    vtkIdType* cellData;
    {
      constructNewArrayIfNecessary(cellData_, cellData, numberOfCells);
      transferDataIfNecessary(cellData_, cellData, numberOfCells);
      deleteOldArrayIfNecessary(cellData_, cellData);
    }

    vtkNew<vtkIdTypeArray> cellDataArray;
    cellDataArray->SetName(domainPropertyName.c_str());
    cellDataArray->SetArray(cellData, numberOfCells, false, vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);
    pd->GetCellData()->AddArray(cellDataArray.GetPointer());

    vtkIdType* pointData;
    {
      constructNewArrayIfNecessary(pointData_, pointData, numberOfPoints);
      transferDataIfNecessary(pointData_, pointData, numberOfPoints);
      deleteOldArrayIfNecessary(pointData_, pointData);
    }

    vtkNew<vtkIdTypeArray> pointDataArray;
    pointDataArray->SetName(domainPropertyName.c_str());
    pointDataArray->SetArray(
      pointData, numberOfPoints, false, vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);
    pd->GetPointData()->AddArray(pointDataArray.GetPointer());
  }

  // CellFields and PointFields are easier than mesh constants because they are
  // required to be set on all of the cells/points in the meshset.
  {
    std::set<smtk::mesh::CellField> cellfields =
      meshset.subset(static_cast<smtk::mesh::DimensionType>(dimension)).cellFields();
    for (auto& cellfield : cellfields)
    {
      double* cellData = new double[cellfield.size() * cellfield.dimension()];
      cellfield.get(cellData);

      vtkNew<vtkDoubleArray> cellDataArray;
      cellDataArray->SetName(cellfield.name().c_str());
      cellDataArray->SetArray(cellData, cellfield.size() * cellfield.dimension(), false,
        vtkDoubleArray::VTK_DATA_ARRAY_DELETE);
      cellDataArray->SetNumberOfComponents(static_cast<int>(cellfield.dimension()));
      pd->GetCellData()->AddArray(cellDataArray.GetPointer());
    }

    std::set<smtk::mesh::PointField> pointfields =
      meshset.subset(static_cast<smtk::mesh::DimensionType>(dimension)).pointFields();
    for (auto& pointfield : pointfields)
    {
      double* pointData = new double[pointfield.size() * pointfield.dimension()];
      pointfield.get(pointData);

      vtkNew<vtkDoubleArray> pointDataArray;
      pointDataArray->SetName(pointfield.name().c_str());
      pointDataArray->SetArray(pointData, pointfield.size() * pointfield.dimension(), false,
        vtkDoubleArray::VTK_DATA_ARRAY_DELETE);
      pointDataArray->SetNumberOfComponents(static_cast<int>(pointfield.dimension()));
      pd->GetPointData()->AddArray(pointDataArray.GetPointer());
    }
  }
}

void ExportVTKData::operator()(
  const smtk::mesh::MeshSet& meshset, vtkUnstructuredGrid* ug, std::string domainPropertyName) const
{
  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //determine the allocation lengths
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(
    meshset, connectivityLength, numberOfCells, numberOfPoints);

  // add the number of cells to the connectivity length to get the length of
  // VTK-style connectivity
  connectivityLength += numberOfCells;

  //create raw data buffers to hold our data
  double* pointsData = new double[3 * numberOfPoints];
  unsigned char* cellTypesData = new unsigned char[numberOfCells];
  std::int64_t* cellLocationsData_ = new std::int64_t[numberOfCells];
  std::int64_t* connectivityData_ = new std::int64_t[connectivityLength];

  //extract tessellation information
  smtk::mesh::PreAllocatedTessellation tess(
    connectivityData_, cellLocationsData_, cellTypesData, pointsData);
  smtk::mesh::extractTessellation(meshset, tess);

  vtkIdType* cellLocationsData;
  {
    constructNewArrayIfNecessary(cellLocationsData_, cellLocationsData, numberOfCells);
    transferDataIfNecessary(cellLocationsData_, cellLocationsData, numberOfCells);
    deleteOldArrayIfNecessary(cellLocationsData_, cellLocationsData);
  }

  vtkIdType* connectivityData;
  {
    constructNewArrayIfNecessary(connectivityData_, connectivityData, connectivityLength);
    transferDataIfNecessary(connectivityData_, connectivityData, connectivityLength);
    deleteOldArrayIfNecessary(connectivityData_, connectivityData);
  }

  // create vtk data arrays to hold our data
  vtkNew<vtkDoubleArray> pointsArray;
  vtkNew<vtkUnsignedCharArray> cellTypes;
  vtkNew<vtkIdTypeArray> cellLocations;
  vtkNew<vtkIdTypeArray> connectivity;

  // transfer ownership of our raw data arrays to the vtk data arrays
  pointsArray->SetNumberOfComponents(3);
  pointsArray->SetArray(
    pointsData, 3 * numberOfPoints, false, vtkDoubleArray::VTK_DATA_ARRAY_DELETE);
  cellTypes->SetArray(
    cellTypesData, numberOfCells, false, vtkUnsignedCharArray::VTK_DATA_ARRAY_DELETE);
  cellLocations->SetArray(
    cellLocationsData, numberOfCells, false, vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);
  connectivity->SetArray(
    connectivityData, connectivityLength, false, vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);

  vtkNew<vtkPoints> points;
  points->SetData(pointsArray.GetPointer());

  vtkNew<vtkCellArray> cells;
  cells->SetCells(numberOfCells, connectivity.GetPointer());

  ug->SetPoints(points.GetPointer());
  ug->SetCells(cellTypes.GetPointer(), cellLocations.GetPointer(), cells.GetPointer());

  if (!domainPropertyName.empty())
  {
    std::int64_t* cellData_ = new std::int64_t[numberOfCells];
    std::int64_t* pointData_ = new std::int64_t[numberOfPoints];

    //extract field information
    smtk::mesh::PreAllocatedMeshConstants meshConstants(cellData_, pointData_);
    smtk::mesh::extractDomainMeshConstants(meshset, meshConstants);

    vtkIdType* cellData;
    {
      constructNewArrayIfNecessary(cellData_, cellData, numberOfCells);
      transferDataIfNecessary(cellData_, cellData, numberOfCells);
      deleteOldArrayIfNecessary(cellData_, cellData);
    }

    vtkNew<vtkIdTypeArray> cellDataArray;
    cellDataArray->SetName(domainPropertyName.c_str());
    cellDataArray->SetArray(cellData, numberOfCells, false, vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);
    ug->GetCellData()->AddArray(cellDataArray.GetPointer());

    vtkIdType* pointData;
    {
      constructNewArrayIfNecessary(pointData_, pointData, numberOfPoints);
      transferDataIfNecessary(pointData_, pointData, numberOfPoints);
      deleteOldArrayIfNecessary(pointData_, pointData);
    }

    vtkNew<vtkIdTypeArray> pointDataArray;
    pointDataArray->SetName(domainPropertyName.c_str());
    pointDataArray->SetArray(
      pointData, numberOfPoints, false, vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);
    ug->GetPointData()->AddArray(pointDataArray.GetPointer());
  }

  // CellFields and PointFields are easier than mesh constants because they are
  // required to be set on all of the cells/points in the meshset.
  {
    std::set<smtk::mesh::CellField> cellfields = meshset.cellFields();
    for (auto& cellfield : cellfields)
    {
      double* cellData = new double[cellfield.size() * cellfield.dimension()];
      cellfield.get(cellData);

      vtkNew<vtkDoubleArray> cellDataArray;
      cellDataArray->SetName(cellfield.name().c_str());
      cellDataArray->SetArray(cellData, cellfield.size() * cellfield.dimension(), false,
        vtkDoubleArray::VTK_DATA_ARRAY_DELETE);
      cellDataArray->SetNumberOfComponents(static_cast<int>(cellfield.dimension()));
      ug->GetCellData()->AddArray(cellDataArray.GetPointer());
    }

    std::set<smtk::mesh::PointField> pointfields = meshset.pointFields();
    for (auto& pointfield : pointfields)
    {
      double* pointData = new double[pointfield.size() * pointfield.dimension()];
      pointfield.get(pointData);

      vtkNew<vtkDoubleArray> pointDataArray;
      pointDataArray->SetName(pointfield.name().c_str());
      pointDataArray->SetArray(pointData, pointfield.size() * pointfield.dimension(), false,
        vtkDoubleArray::VTK_DATA_ARRAY_DELETE);
      pointDataArray->SetNumberOfComponents(static_cast<int>(pointfield.dimension()));
      ug->GetPointData()->AddArray(pointDataArray.GetPointer());
    }
  }
}
}
}
}
}
