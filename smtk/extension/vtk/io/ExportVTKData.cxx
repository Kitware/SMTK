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

#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/CellTraits.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/ExtractField.h"
#include "smtk/mesh/ExtractTessellation.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/MeshSet.h"

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

#include "smtk/mesh/moab/Interface.h"
#include "smtk/mesh/moab/CellTypeToType.h"

namespace smtk {
namespace extension {
namespace vtk {
namespace io {

ExportVTKData::ExportVTKData()
{

}

//----------------------------------------------------------------------------
bool ExportVTKData::operator()(const std::string& filename,
                               smtk::mesh::CollectionPtr collection,
                               std::string domainPropertyName) const
{
  std::string extension =
    vtksys::SystemTools::GetFilenameLastExtension(filename.c_str());

  // Dispatch based on the file extension
  if (extension == ".vtu")
   {
   vtkSmartPointer<vtkUnstructuredGrid> ug =
     vtkSmartPointer<vtkUnstructuredGrid>::New();
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

//----------------------------------------------------------------------------
namespace
{
template<class T, class U>
struct swapDataTypesIfNecessary
{
  void operator()(T*& in, U*& out, boost::int64_t len)
  {
    out = new U[len];
    for (boost::int64_t i=0;i<len;i++)
      {
      out[i] = in[i];
      }
    delete [] in;
  }
};

template<class T>
struct swapDataTypesIfNecessary<T,T>
{
  void operator()(T*& in, T*& out, boost::int64_t)
  {
    out = in;
  }
};

}

//----------------------------------------------------------------------------
void ExportVTKData::operator()(const smtk::mesh::MeshSet& meshset,
                               vtkPolyData* pd,
                               std::string domainPropertyName) const
{
  // We are only getting the highest dimension cells starting Dims2
  smtk::mesh::CellSet cells = meshset.cells(smtk::mesh::Dims2);

  if( cells.is_empty() == true)
    {
    cells = meshset.cells(smtk::mesh::Dims1);
    }
  if( cells.is_empty() == true)
    {
    cells = meshset.cells(smtk::mesh::Dims0);
    }

  vtkNew<vtkPoints> pts;
  pd->SetPoints(pts.GetPointer());

  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(
    cells, connectivityLength, numberOfCells, numberOfPoints);

  // cell connectivity
  vtkNew<vtkCellArray> cellarray;

  // points coordinates
  pts->SetDataTypeToDouble();

  if(numberOfPoints == 1)
    {
    double xyz[3];
    cells.points().get(xyz);
    pts->InsertNextPoint(xyz);
    vtkNew<vtkIdList> ptids;
    ptids->InsertNextId(0);
    cellarray->InsertNextCell(ptids.GetPointer());
    pd->SetVerts(cellarray.GetPointer());
    }
  else
    {
    pts->SetNumberOfPoints(numberOfPoints);
    double *rawPoints = static_cast<double*>(pts->GetVoidPointer(0));

    cellarray->Allocate(connectivityLength + numberOfCells);
    boost::int64_t* cellconn = reinterpret_cast<boost::int64_t *>(
                cellarray->WritePointer(numberOfCells, connectivityLength + numberOfCells));
    smtk::mesh::PreAllocatedTessellation tess(cellconn,
                                              rawPoints);

    smtk::mesh::extractTessellation(cells, tess);
    smtk::mesh::CellTypes ctypes = cells.types().cellTypes();

    if (ctypes[smtk::mesh::Triangle]
      || ctypes[smtk::mesh::Quad]
      || ctypes[smtk::mesh::Polygon]
      )
      {
      pd->SetPolys(cellarray.GetPointer());
      }
    else if (ctypes[smtk::mesh::Line])
      {
      pd->SetLines(cellarray.GetPointer());
      }
    else if (ctypes[smtk::mesh::Vertex])
      {
      pd->SetVerts(cellarray.GetPointer());
      }
    }

  if (!domainPropertyName.empty())
    {
    boost::int64_t* cellData_ = new boost::int64_t[numberOfCells];
    boost::int64_t* pointData_ = new boost::int64_t[numberOfPoints];

    //extract field information
    smtk::mesh::PreAllocatedField field(cellData_, pointData_);
    smtk::mesh::extractDomainField(meshset, field);

    swapDataTypesIfNecessary<boost::int64_t, vtkIdType> swap;

    vtkIdType* cellData;
    swap(cellData_, cellData, numberOfCells);

    vtkNew<vtkIdTypeArray> cellDataArray;
    cellDataArray->SetName(domainPropertyName.c_str());
    cellDataArray->SetArray(cellData, numberOfCells, false,
                            vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);
    pd->GetCellData()->AddArray(cellDataArray.GetPointer());

    vtkIdType* pointData;
    swap(pointData_, pointData, numberOfPoints);

    vtkNew<vtkIdTypeArray> pointDataArray;
    pointDataArray->SetName(domainPropertyName.c_str());
    pointDataArray->SetArray(pointData, numberOfPoints, false,
                            vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);
    pd->GetPointData()->AddArray(pointDataArray.GetPointer());
    }
}

//----------------------------------------------------------------------------
void ExportVTKData::operator()(const smtk::mesh::MeshSet& meshset,
                               vtkUnstructuredGrid* ug,
                               std::string domainPropertyName) const
{
  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  //determine the allocation lengths
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(
    meshset, connectivityLength, numberOfCells, numberOfPoints);

  // add the number of cells to the connectivity length to get the length of
  // VTK-style connectivity
  connectivityLength += numberOfCells;

  //create raw data buffers to hold our data
  double* pointsData = new double[3*numberOfPoints];
  unsigned char* cellTypesData = new unsigned char[numberOfCells];
  boost::int64_t* cellLocationsData_ = new boost::int64_t[numberOfCells];
  boost::int64_t* connectivityData_ = new boost::int64_t[connectivityLength];

  //extract tessellation information
  smtk::mesh::PreAllocatedTessellation tess(connectivityData_,
                                            cellLocationsData_,
                                            cellTypesData, pointsData);
  smtk::mesh::extractTessellation(meshset, tess);

  swapDataTypesIfNecessary<boost::int64_t, vtkIdType> swap;
  vtkIdType* cellLocationsData;
  swap(cellLocationsData_, cellLocationsData, numberOfCells);
  vtkIdType* connectivityData;
  swap(connectivityData_, connectivityData, connectivityLength);

  // create vtk data arrays to hold our data
  vtkNew<vtkDoubleArray> pointsArray;
  vtkNew<vtkUnsignedCharArray> cellTypes;
  vtkNew<vtkIdTypeArray> cellLocations;
  vtkNew<vtkIdTypeArray> connectivity;

  // transfer ownership of our raw data arrays to the vtk data arrays
  pointsArray->SetNumberOfComponents(3);
  pointsArray->SetArray(pointsData, 3*numberOfPoints, false,
                        vtkDoubleArray::VTK_DATA_ARRAY_DELETE);
  cellTypes->SetArray(cellTypesData, numberOfCells, false,
                      vtkUnsignedCharArray::VTK_DATA_ARRAY_DELETE);
  cellLocations->SetArray(cellLocationsData, numberOfCells, false,
                          vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);
  connectivity->SetArray(connectivityData, connectivityLength, false,
                         vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);

  vtkNew<vtkPoints> points;
  points->SetData(pointsArray.GetPointer());

  vtkNew<vtkCellArray> cells;
  cells->SetCells(numberOfCells, connectivity.GetPointer());

  ug->SetPoints(points.GetPointer());
  ug->SetCells(cellTypes.GetPointer(), cellLocations.GetPointer(),
               cells.GetPointer());

  if (!domainPropertyName.empty())
    {
    boost::int64_t* cellData_ = new boost::int64_t[numberOfCells];
    boost::int64_t* pointData_ = new boost::int64_t[numberOfPoints];

    //extract field information
    smtk::mesh::PreAllocatedField field(cellData_, pointData_);
    smtk::mesh::extractDomainField(meshset, field);

    swapDataTypesIfNecessary<boost::int64_t, vtkIdType> swap;

    vtkIdType* cellData;
    swap(cellData_, cellData, numberOfCells);

    vtkNew<vtkIdTypeArray> cellDataArray;
    cellDataArray->SetName(domainPropertyName.c_str());
    cellDataArray->SetArray(cellData, numberOfCells, false,
                            vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);
    ug->GetCellData()->AddArray(cellDataArray.GetPointer());

    vtkIdType* pointData;
    swap(pointData_, pointData, numberOfPoints);

    vtkNew<vtkIdTypeArray> pointDataArray;
    pointDataArray->SetName(domainPropertyName.c_str());
    pointDataArray->SetArray(pointData, numberOfPoints, false,
                            vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);
    ug->GetPointData()->AddArray(pointDataArray.GetPointer());
    }
}

}
}
}
}
