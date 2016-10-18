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

#include "smtk/extension/vtk/io/VTKDataConverter.h"

#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/CellTraits.h"
#include "smtk/mesh/Collection.h"
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

namespace detail
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

//----------------------------------------------------------------------------
bool convertVTKPoints(vtkPoints* points,
                      const smtk::mesh::AllocatorPtr& ialloc,
                      smtk::mesh::Handle& firstVertHandle)
{
  const vtkIdType numberOfPoints = points->GetNumberOfPoints();
  std::vector<double *> coords;
  const bool pointsAllocated = ialloc->allocatePoints( numberOfPoints,
                                                       firstVertHandle,
                                                       coords);
  if(pointsAllocated)
    {
    double point[3];
    for(vtkIdType i = 0; i < numberOfPoints; ++i)
      {
      //note this could become a performance bottleneck. If that occurs
      //we will need to move to a template dispatch solution to handle floats,
      //doubles, and vtk Mapped Arrays
      points->GetPoint(i, point);
      coords[0][i] = point[0];
      coords[1][i] = point[1];
      coords[2][i] = point[2];
      }
    }
  return pointsAllocated;
}

//----------------------------------------------------------------------------
template<typename VTKDataSetType>
bool convertVTKCells( VTKDataSetType* dataset,
                      const smtk::mesh::AllocatorPtr& ialloc,
                      smtk::mesh::Handle firstVertHandle,
                      smtk::mesh::HandleRange& newlyCreatedCells)
{
  //iterate the dataset collecting cells of the same
  //time intill we hit the end or find a different cell type
  //This is done to improve the allocation and insertion performance.

  const vtkIdType numberOfVTKCells = dataset->GetNumberOfCells();
  std::size_t numberOfCellsConverted = 0;

  vtkIdType currentCellId = 0;
  while( currentCellId < numberOfVTKCells)
    {
    vtkIdType startOfContinousCellIds = currentCellId;
    vtkIdType endOfContinousCellIds = currentCellId + 1;

    const int currentVTKCellType = dataset->GetCellType( startOfContinousCellIds );
    //we need to convert from a vtk cell type to a smtk::mesh cell type
    const smtk::mesh::CellType cellType = vtkToSMTKCell( currentVTKCellType );
    if(cellType == smtk::mesh::CellType_MAX)
      {
      //we hit a vtk cell type we don't support, skip to the next section
      currentCellId = endOfContinousCellIds;
      continue;
      }

    //keep iterating while we have the same cell type. The only exception
    //is POLYGON
    const vtkIdType numVertsPerCell = dataset->GetCell( startOfContinousCellIds )->GetNumberOfPoints();
    if(cellType != smtk::mesh::Polygon)
      {
      while( endOfContinousCellIds < numberOfVTKCells &&
            currentVTKCellType == dataset->GetCellType( endOfContinousCellIds ) )
        { ++endOfContinousCellIds; }
      }
    else
      {
      //for polygon we need to continue while the number of points are
      //the same
      vtkIdType end_npts = dataset->GetCell( endOfContinousCellIds )->GetNumberOfPoints();
      while( endOfContinousCellIds < numberOfVTKCells &&
             currentVTKCellType == dataset->GetCellType( endOfContinousCellIds ) &&
             numVertsPerCell == end_npts)
        {
        end_npts = dataset->GetCell( endOfContinousCellIds )->GetNumberOfPoints();
        ++endOfContinousCellIds;
        }
      }

    //now we know the start(inclusive) and end(exclusive) index.
    //allocate the moab data
    const std::size_t numCellsToAlloc = static_cast<std::size_t>(endOfContinousCellIds - startOfContinousCellIds);

    //need to convert from vtk cell type to moab cell type
    bool allocated = false;
    smtk::mesh::Handle *startOfConnectivityArray = 0;

    //only convert cells smtk mesh supports
    smtk::mesh::HandleRange cellsCreatedForThisType;
    allocated = ialloc->allocateCells( cellType,
                                       numCellsToAlloc,
                                       static_cast<int>(numVertsPerCell),
                                       cellsCreatedForThisType,
                                       startOfConnectivityArray);
    if(allocated)
      {
      //now that we have the chunk allocated need to fill it
      //we do this by iterating the cells
      vtkIdType npts, *pts;
      smtk::mesh::Handle *currentConnLoc = startOfConnectivityArray;
      for( vtkIdType i = startOfContinousCellIds; i < endOfContinousCellIds; ++i )
        {
        dataset->GetCellPoints( i, npts, pts );
        //currently only supports linear elements
        for(vtkIdType j=0; j < npts; ++j)
          {
          currentConnLoc[j] = firstVertHandle + pts[j];
          }
        currentConnLoc += npts;
        }

      // notify database that we have written to connectivity, that way
      // it can properly update adjacencies and other database info
      ialloc->connectivityModified(newlyCreatedCells,
                                   static_cast<int>(numVertsPerCell),
                                   startOfConnectivityArray);

      //update the number of cells that we have converted to smtk::mesh
      numberOfCellsConverted += numCellsToAlloc;

      //insert these cells back into the range
      newlyCreatedCells.insert(cellsCreatedForThisType.begin(),
                               cellsCreatedForThisType.end());
      }

    //increment the currentCellId to the end of the current continous cell
    //block so that we can find the next series of cells
    currentCellId = endOfContinousCellIds;
    }

  return numberOfCellsConverted != 0;
}

//----------------------------------------------------------------------------
bool convertDomain(vtkCellData* cellData,
                   const smtk::mesh::InterfacePtr& iface,
                   const smtk::mesh::HandleRange& cells,
                   const std::string& materialPropertyName)
{
  if(cellData == NULL)
    {
    //we have no information
    return false;
    }

  vtkDataArray* materialData = cellData->GetArray( materialPropertyName.c_str() );
  if(!materialData || materialData->GetNumberOfComponents() != 1)
    { //needs to be a scalar array
    return false;
    }

  if(materialData->GetNumberOfTuples() != static_cast<int>(cells.size()))
    { //we currently don't support applying material when
      //we only loaded in some of the cells
    return false;
    }

  std::map< int, smtk::mesh::HandleRange > meshes;
  //move each cell from the entire pool, into a range
  //that represents that material mesh. This is slowish.

  typedef smtk::mesh::HandleRange::const_iterator cit;
  vtkIdType index = 0;
  for(cit i=cells.begin(); i!=cells.end(); ++i, ++index)
    {
    const int currentMaterial = static_cast<int>(materialData->GetTuple1(index));
    meshes[currentMaterial].insert(*i);
    }

  typedef std::map< int, smtk::mesh::HandleRange >::const_iterator map_cit;
  for(map_cit i=meshes.begin(); i != meshes.end(); ++i)
    {
    smtk::mesh::Handle meshId;
    const bool created = iface->createMesh(i->second, meshId);
    if(created)
      {
      smtk::mesh::HandleRange meshHandles;
      meshHandles.insert(meshId);
      //assign a material id to the mesh
      iface->setDomain(meshHandles, smtk::mesh::Domain(i->first));
      }
    }

   return true;
}

}

//----------------------------------------------------------------------------
VTKDataConverter::VTKDataConverter()
{

}

//-------------------------------------------------------------------------
namespace
{
template<typename TReader>
vtkDataSet* readXMLFile(const std::string& fileName)
{
  vtkSmartPointer<TReader> reader = vtkSmartPointer<TReader>::New();
  reader->SetFileName(fileName.c_str());
  reader->Update();
  reader->GetOutput()->Register(reader);
  return vtkDataSet::SafeDownCast(reader->GetOutput());
}
}

//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr
VTKDataConverter::operator()(const std::string& filename,
                             smtk::mesh::ManagerPtr& manager,
                             std::string materialPropertyName) const
{
  smtk::mesh::CollectionPtr collection = manager->makeCollection();
  return this->operator()(filename, collection, materialPropertyName) ?
    collection : smtk::mesh::CollectionPtr();
}

//----------------------------------------------------------------------------
bool VTKDataConverter::operator()(const std::string& filename,
                                  smtk::mesh::CollectionPtr collection,
                                  std::string materialPropertyName) const
{
  std::string extension =
    vtksys::SystemTools::GetFilenameLastExtension(filename.c_str());

  // Dispatch based on the file extension
  vtkDataSet* data;
  smtk::mesh::CollectionPtr c;
  if (extension == ".vtu")
   {
   data = readXMLFile<vtkXMLUnstructuredGridReader> (filename);
   return this->operator()(vtkUnstructuredGrid::SafeDownCast(data),
                           collection,
                           materialPropertyName);
   }
  else if (extension == ".vtp")
   {
   data = readXMLFile<vtkXMLPolyDataReader> (filename);
   return this->operator()(vtkPolyData::SafeDownCast(data),
                           collection,
                           materialPropertyName);
   }

  return false;
}

//----------------------------------------------------------------------------
bool VTKDataConverter::operator()(vtkPolyData* polydata,
                                  smtk::mesh::CollectionPtr collection,
                                  std::string materialPropertyName) const
{
  //make sure we have a valid poly data
  if(!polydata)
    {
    return false;
    }
  else if( polydata->GetNumberOfPoints() == 0 ||
           polydata->GetNumberOfCells() == 0)
    {
    //early terminate if the polydata is empty.
    return false;
    }

  bool pointsConverted = false;
  bool cellsConverted = false;
  bool meshCreated = false;

  //allocate space for coordinates and load them into moab
  smtk::mesh::InterfacePtr iface = collection->interface();
  smtk::mesh::AllocatorPtr ialloc = iface->allocator();

  smtk::mesh::Handle firstVertHandle;
  pointsConverted = detail::convertVTKPoints(polydata->GetPoints(),
                                             ialloc,
                                             firstVertHandle);


  //allocate and fill connectivity
  smtk::mesh::HandleRange newlyCreatedCells;
  cellsConverted = detail::convertVTKCells(polydata,
                                           ialloc,
                                           firstVertHandle,
                                           newlyCreatedCells );

  if(pointsConverted && cellsConverted)
    {
    if(materialPropertyName.empty())
      { //if we don't have a material we create a single mesh
      smtk::mesh::Handle vtkMeshHandle;
      meshCreated = iface->createMesh(newlyCreatedCells, vtkMeshHandle);
      }
    else
      { //make multiple meshes each one assigned a material value
      meshCreated = detail::convertDomain(polydata->GetCellData(),
                                          iface,
                                          newlyCreatedCells,
                                          materialPropertyName);
      }
    }

  return meshCreated;
}

//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr
VTKDataConverter::operator()(vtkPolyData* polydata,
                             smtk::mesh::ManagerPtr& manager,
                             std::string materialPropertyName) const
{
  smtk::mesh::CollectionPtr c = manager->makeCollection();
  return this->operator()(polydata,c,materialPropertyName) ? c : smtk::mesh::CollectionPtr();
}

//----------------------------------------------------------------------------
bool VTKDataConverter::operator()(vtkUnstructuredGrid* ugrid,
                                  smtk::mesh::CollectionPtr collection,
                                  std::string materialPropertyName) const
{
  //make sure we have a valid poly data
  if(!ugrid)
    {
    return false;
    }
  else if( ugrid->GetNumberOfPoints() == 0 ||
           ugrid->GetNumberOfCells() == 0)
    {
    //early terminate if the ugrid is empty.
    return false;
    }

  bool pointsConverted = false;
  bool cellsConverted = false;
  bool meshCreated = false;

  //allocate space for coordinates and load them into moab
  smtk::mesh::InterfacePtr iface = collection->interface();
  smtk::mesh::AllocatorPtr ialloc = iface->allocator();

  smtk::mesh::Handle firstVertHandle;
  pointsConverted = detail::convertVTKPoints(ugrid->GetPoints(),
                                             ialloc,
                                             firstVertHandle);

  //allocate and fill connectivity
  smtk::mesh::HandleRange newlyCreatedCells;
  cellsConverted = detail::convertVTKCells(ugrid,
                                           ialloc,
                                           firstVertHandle,
                                           newlyCreatedCells );

  if(pointsConverted && cellsConverted)
    {
    if(materialPropertyName.empty())
      { //if we don't have a material we create a single mesh
      smtk::mesh::Handle vtkMeshHandle;
      meshCreated = iface->createMesh(newlyCreatedCells, vtkMeshHandle);
      }
    else
      { //make multiple meshes each one assigned a material value
      meshCreated = detail::convertDomain(ugrid->GetCellData(),
                                          iface,
                                          newlyCreatedCells,
                                          materialPropertyName);
      }
    }
  return meshCreated;
}

//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr
VTKDataConverter::operator()(vtkUnstructuredGrid* ugrid,
                             smtk::mesh::ManagerPtr& manager,
                             std::string materialPropertyName) const
{
  smtk::mesh::CollectionPtr c = manager->makeCollection();
  return this->operator()(ugrid,c,materialPropertyName) ? c : smtk::mesh::CollectionPtr();
}

//----------------------------------------------------------------------------
bool VTKDataConverter::operator()(const std::string& filename,
                                  smtk::mesh::CollectionPtr collection) const
{
  std::string extension =
    vtksys::SystemTools::GetFilenameLastExtension(filename.c_str());

  // Dispatch based on the file extension
  if (extension == ".vtu")
   {
   vtkSmartPointer<vtkUnstructuredGrid> ug =
     vtkSmartPointer<vtkUnstructuredGrid>::New();
   this->operator()(collection->meshes(), ug);
   vtkNew<vtkXMLUnstructuredGridWriter> writer;
   writer->SetFileName(filename.c_str());
   writer->SetInputData(ug);
   writer->Write();
   return true;
   }
  else if (extension == ".vtp")
   {
   vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
   this->operator()(collection->meshes(), pd);
   vtkNew<vtkXMLPolyDataWriter> writer;
   writer->SetFileName(filename.c_str());
   writer->SetInputData(pd);
   writer->Write();
   return true;
   }

  return false;
}

//----------------------------------------------------------------------------
void VTKDataConverter::operator()(const smtk::mesh::MeshSet& meshset,
                                  vtkPolyData* pd) const
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
}

//----------------------------------------------------------------------------
void VTKDataConverter::operator()(const smtk::mesh::MeshSet& meshset,
                                  vtkUnstructuredGrid* ug) const
{
  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  //determine the allocation lengths
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(
    meshset, connectivityLength, numberOfCells, numberOfPoints);

  //create raw data buffers to hold our data
  double* pointsData = new double[3*numberOfPoints];
  unsigned char* cellTypesData = new unsigned char[numberOfCells];
  vtkIdType* cellLocationsData = new vtkIdType[numberOfCells];
  vtkIdType* connectivityData = new vtkIdType[connectivityLength+numberOfCells];

  //extract tessellation information
  smtk::mesh::PreAllocatedTessellation tess(connectivityData, cellLocationsData,
                                            cellTypesData, pointsData);
  smtk::mesh::extractTessellation(meshset, tess);

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
}

}
}
}
}
