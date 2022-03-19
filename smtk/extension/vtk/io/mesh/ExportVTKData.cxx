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

#include "smtk/extension/vtk/io/mesh/ExportVTKData.h"
#include "smtk/extension/vtk/io/mesh/MeshIOVTK.h"

#include "smtk/mesh/core/CellField.h"
#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/CellTraits.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/PointField.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/utility/ExtractMeshConstants.h"
#include "smtk/mesh/utility/ExtractTessellation.h"
#include "smtk/mesh/utility/Metrics.h"

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
#include "vtkUnstructuredGridWriter.h"
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

ExportVTKData::ExportVTKData() = default;

bool ExportVTKData::operator()(
  const std::string& filename,
  smtk::mesh::ResourcePtr resource,
  std::string domainPropertyName) const
{
  // fail if the resource is empty
  if (!resource || !resource->isValid())
  {
    return false;
  }

  return (*this)(filename, resource->meshes(), domainPropertyName);
}

bool ExportVTKData::operator()(
  const std::string& filename,
  const smtk::mesh::MeshSet& meshset,
  std::string domainPropertyName) const
{
  // fail if the meshset is empty
  if (meshset.is_empty())
  {
    return false;
  }

  std::string extension = vtksys::SystemTools::GetFilenameLastExtension(filename);

  // Dispatch based on the file extension
  if (extension == ".vtu")
  {
    vtkSmartPointer<vtkUnstructuredGrid> ug = vtkSmartPointer<vtkUnstructuredGrid>::New();
    this->operator()(meshset, ug, domainPropertyName);
    vtkNew<vtkXMLUnstructuredGridWriter> writer;
    writer->SetFileName(filename.c_str());
    writer->SetInputData(ug);
    writer->Write();
    return true;
  }
  else if (extension == ".vtp")
  {
    vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
    this->operator()(meshset, pd, domainPropertyName);
    vtkNew<vtkXMLPolyDataWriter> writer;
    writer->SetFileName(filename.c_str());
    writer->SetInputData(pd);
    writer->Write();
    return true;
  }
  else if (extension == ".vtk")
  {
    vtkSmartPointer<vtkUnstructuredGrid> ug = vtkSmartPointer<vtkUnstructuredGrid>::New();
    this->operator()(meshset, ug, domainPropertyName);
    vtkNew<vtkUnstructuredGridWriter> writer;
    writer->SetFileName(filename.c_str());
    writer->SetInputData(ug);
    writer->Write();
    return true;
  }

  return false;
}

namespace
{

// functions for allocation, transfer and deallocation when there is a type
// mismatch
template<typename T>
void constructNewArrayIfNecessary(T*& /*unused*/, vtkIdType*& out, std::int64_t len)
{
  out = new vtkIdType[len];
}

template<typename T>
void transferDataIfNecessary(T*& in, vtkIdType*& out, std::int64_t len)
{
  for (std::int64_t i = 0; i < len; i++)
  {
    out[i] = in[i];
  }
}

template<typename T>
void deleteOldArrayIfNecessary(T*& in, vtkIdType*& /*unused*/)
{
  delete[] in;
}
} // namespace

void ExportVTKData::operator()(
  const smtk::mesh::MeshSet& meshset,
  vtkPolyData* pd,
  std::string domainPropertyName) const
{
  // Determine the highest dimension
  int dimension = smtk::mesh::utility::highestDimension(meshset);

  if (dimension < 0)
  {
    // We have been passed a meshset with no elements of dimension 3 or lower.
    return;
  }

  // To preserve the state of the mesh database, we track
  // whether or not a new meshset was created to represent
  // the 3d shell; if it was created, we delete it when we
  // are finished with it.
  bool shellCreated = false;
  smtk::mesh::MeshSet toRender = (dimension == 3 ? meshset.extractShell(shellCreated) : meshset);

  smtk::mesh::CellSet cellset =
    toRender.cells(static_cast<smtk::mesh::DimensionType>(dimension == 3 ? 2 : dimension));

  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //determine the allocation lengths
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    cellset, connectivityLength, numberOfCells, numberOfPoints);

  // add the number of cells to the connectivity length to get the length of
  // VTK-style connectivity
  connectivityLength += numberOfCells;

  //create raw data buffers to hold our data
  double* pointsData = new double[3 * numberOfPoints];
  std::int64_t* connectivityData_ = new std::int64_t[connectivityLength];

  //extract tessellation information
  smtk::mesh::utility::PreAllocatedTessellation tess(connectivityData_, pointsData);
  smtk::mesh::utility::extractTessellation(cellset, tess);

  vtkIdType* connectivityData;
  {
    constructNewArrayIfNecessary(connectivityData_, connectivityData, connectivityLength);
    transferDataIfNecessary(connectivityData_, connectivityData, connectivityLength);
    deleteOldArrayIfNecessary(connectivityData_, connectivityData);
  }

  std::int64_t* cellHandles_ = new std::int64_t[numberOfCells];
  if (dimension == 3)
  {
    auto interface = meshset.resource()->interface();

    const smtk::mesh::HandleRange& range = cellset.range();
    auto it = smtk::mesh::rangeElementsBegin(range);
    auto end = smtk::mesh::rangeElementsEnd(range);
    smtk::mesh::Handle parent;
    int canonicalIndex;
    for (std::size_t counter = 0; it != end; ++it, ++counter)
    {
      interface->canonicalIndex(*it, parent, canonicalIndex);
      cellHandles_[counter] = parent;
    }
  }
  else
  {
    const smtk::mesh::HandleRange& range = cellset.range();
    auto it = smtk::mesh::rangeElementsBegin(range);
    auto end = smtk::mesh::rangeElementsEnd(range);
    for (std::size_t counter = 0; it != end; ++it, ++counter)
    {
      cellHandles_[counter] = *it;
    }
  }
  vtkIdType* cellHandles;
  {
    constructNewArrayIfNecessary(cellHandles_, cellHandles, numberOfCells);
    transferDataIfNecessary(cellHandles_, cellHandles, numberOfCells);
    deleteOldArrayIfNecessary(cellHandles_, cellHandles);
  }

  std::int64_t* pointHandles_ = new std::int64_t[numberOfPoints];
  {
    smtk::mesh::PointSet points = cellset.points();
    const smtk::mesh::HandleRange& range = points.range();

    auto it = smtk::mesh::rangeElementsBegin(range);
    auto end = smtk::mesh::rangeElementsEnd(range);
    for (std::size_t counter = 0; it != end; ++it, ++counter)
    {
      pointHandles_[counter] = *it;
    }
  }
  vtkIdType* pointHandles;
  {
    constructNewArrayIfNecessary(pointHandles_, pointHandles, numberOfPoints);
    transferDataIfNecessary(pointHandles_, pointHandles, numberOfPoints);
    deleteOldArrayIfNecessary(pointHandles_, pointHandles);
  }

  // create vtk data arrays to hold our data
  vtkNew<vtkDoubleArray> pointsArray;
  vtkNew<vtkIdTypeArray> connectivity;

  // transfer ownership of our raw data arrays to the vtk data arrays
  pointsArray->SetNumberOfComponents(3);
  pointsArray->SetArray(
    pointsData, 3 * numberOfPoints, false, vtkDoubleArray::VTK_DATA_ARRAY_DELETE);
  connectivity->SetArray(
    connectivityData, connectivityLength, false, vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);

  vtkNew<vtkPoints> points;
  points->SetData(pointsArray.GetPointer());
  pd->SetPoints(points.GetPointer());

  vtkNew<vtkCellArray> cells;
  cells->SetCells(numberOfCells, connectivity.GetPointer());

  if (dimension == 3 || dimension == 2)
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

  vtkNew<vtkIdTypeArray> cellHandlesArray;
  cellHandlesArray->SetName(smtk::extension::vtk::io::mesh::MeshIOVTK::CellHandlesName);
  cellHandlesArray->SetArray(
    cellHandles, numberOfCells, false, vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);
  pd->GetCellData()->AddArray(cellHandlesArray.GetPointer());

  vtkNew<vtkIdTypeArray> pointHandlesArray;
  pointHandlesArray->SetName(smtk::extension::vtk::io::mesh::MeshIOVTK::PointHandlesName);
  pointHandlesArray->SetArray(
    pointHandles, numberOfPoints, false, vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);
  pd->GetPointData()->AddArray(pointHandlesArray.GetPointer());

  if (!domainPropertyName.empty())
  {
    std::int64_t* cellData_ = new std::int64_t[numberOfCells];
    std::int64_t* pointData_ = new std::int64_t[numberOfPoints];

    //extract mesh constant information
    smtk::mesh::utility::PreAllocatedMeshConstants meshConstants(cellData_, pointData_);
    smtk::mesh::utility::extractDomainMeshConstants(toRender, meshConstants);

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
      toRender.subset(static_cast<smtk::mesh::DimensionType>(dimension)).cellFields();
    for (const auto& cellfield : cellfields)
    {
      if (cellfield.type() == smtk::mesh::FieldType::Double)
      {
        double* cellData = new double[cellfield.size() * cellfield.dimension()];
        cellfield.get(cellData);

        vtkNew<vtkDoubleArray> cellDataArray;
        cellDataArray->SetName(cellfield.name().c_str());
        cellDataArray->SetArray(
          cellData,
          cellfield.size() * cellfield.dimension(),
          false,
          vtkDoubleArray::VTK_DATA_ARRAY_DELETE);
        cellDataArray->SetNumberOfComponents(static_cast<int>(cellfield.dimension()));
        pd->GetCellData()->AddArray(cellDataArray.GetPointer());
      }
      else if (cellfield.type() == smtk::mesh::FieldType::Integer)
      {
        int* cellData = new int[cellfield.size() * cellfield.dimension()];
        cellfield.get(cellData);

        vtkNew<vtkIntArray> cellDataArray;
        cellDataArray->SetName(cellfield.name().c_str());
        cellDataArray->SetArray(
          cellData,
          cellfield.size() * cellfield.dimension(),
          false,
          vtkIntArray::VTK_DATA_ARRAY_DELETE);
        cellDataArray->SetNumberOfComponents(static_cast<int>(cellfield.dimension()));
        pd->GetCellData()->AddArray(cellDataArray.GetPointer());
      }
    }

    std::set<smtk::mesh::PointField> pointfields =
      toRender.subset(static_cast<smtk::mesh::DimensionType>(dimension)).pointFields();
    for (const auto& pointfield : pointfields)
    {
      if (pointfield.type() == smtk::mesh::FieldType::Double)
      {
        double* pointData = new double[pointfield.size() * pointfield.dimension()];
        pointfield.get(pointData);

        vtkNew<vtkDoubleArray> pointDataArray;
        pointDataArray->SetName(pointfield.name().c_str());
        pointDataArray->SetArray(
          pointData,
          pointfield.size() * pointfield.dimension(),
          false,
          vtkDoubleArray::VTK_DATA_ARRAY_DELETE);
        pointDataArray->SetNumberOfComponents(static_cast<int>(pointfield.dimension()));
        pd->GetPointData()->AddArray(pointDataArray.GetPointer());
      }
      else if (pointfield.type() == smtk::mesh::FieldType::Integer)
      {
        int* pointData = new int[pointfield.size() * pointfield.dimension()];
        pointfield.get(pointData);

        vtkNew<vtkIntArray> pointDataArray;
        pointDataArray->SetName(pointfield.name().c_str());
        pointDataArray->SetArray(
          pointData,
          pointfield.size() * pointfield.dimension(),
          false,
          vtkIntArray::VTK_DATA_ARRAY_DELETE);
        pointDataArray->SetNumberOfComponents(static_cast<int>(pointfield.dimension()));
        pd->GetPointData()->AddArray(pointDataArray.GetPointer());
      }
    }
  }

  if (shellCreated)
  {
    toRender.resource()->removeMeshes(toRender);
  }
}

void ExportVTKData::operator()(
  const smtk::mesh::MeshSet& meshset,
  vtkUnstructuredGrid* ug,
  std::string domainPropertyName) const
{
  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //determine the allocation lengths
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
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
  smtk::mesh::utility::PreAllocatedTessellation tess(
    connectivityData_, cellLocationsData_, cellTypesData, pointsData);
  smtk::mesh::utility::extractTessellation(meshset, tess);

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
    smtk::mesh::utility::PreAllocatedMeshConstants meshConstants(cellData_, pointData_);
    smtk::mesh::utility::extractDomainMeshConstants(meshset, meshConstants);

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
    for (const auto& cellfield : cellfields)
    {
      double* cellData = new double[cellfield.size() * cellfield.dimension()];
      cellfield.get(cellData);

      vtkNew<vtkDoubleArray> cellDataArray;
      cellDataArray->SetName(cellfield.name().c_str());
      cellDataArray->SetArray(
        cellData,
        cellfield.size() * cellfield.dimension(),
        false,
        vtkDoubleArray::VTK_DATA_ARRAY_DELETE);
      cellDataArray->SetNumberOfComponents(static_cast<int>(cellfield.dimension()));
      ug->GetCellData()->AddArray(cellDataArray.GetPointer());
    }

    std::set<smtk::mesh::PointField> pointfields = meshset.pointFields();
    for (const auto& pointfield : pointfields)
    {
      double* pointData = new double[pointfield.size() * pointfield.dimension()];
      pointfield.get(pointData);

      vtkNew<vtkDoubleArray> pointDataArray;
      pointDataArray->SetName(pointfield.name().c_str());
      pointDataArray->SetArray(
        pointData,
        pointfield.size() * pointfield.dimension(),
        false,
        vtkDoubleArray::VTK_DATA_ARRAY_DELETE);
      pointDataArray->SetNumberOfComponents(static_cast<int>(pointfield.dimension()));
      ug->GetPointData()->AddArray(pointDataArray.GetPointer());
    }
  }
}
} // namespace mesh
} // namespace io
} // namespace vtk
} // namespace extension
} // namespace smtk
