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

#include "smtk/extension/vtkToMesh/VTKDataConverter.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/CellTraits.h"

#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkPoints.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkIntArray.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLUnstructuredGridReader.h"

#include "vtksys/SystemTools.hxx"

#include "moab/ReadUtilIface.hpp"

#include "smtk/mesh/moab/Interface.h"
#include "smtk/mesh/moab/CellTypeToType.h"

namespace smtk {
namespace extension {
namespace vtkToMesh {

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
VTKDataConverter::VTKDataConverter(const smtk::mesh::ManagerPtr& manager):
  m_manager( manager )
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
VTKDataConverter::operator()(std::string& filename,
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
                           materialPropertyName);
   }
  else if (extension == ".vtp")
   {
   data = readXMLFile<vtkXMLPolyDataReader> (filename);
   return this->operator()(vtkPolyData::SafeDownCast(data),
                           materialPropertyName);
   }

  return c;
}

//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr
VTKDataConverter::operator()(vtkPolyData* polydata,
                             std::string materialPropertyName) const
{
  smtk::mesh::CollectionPtr nullCollectionPtr;
  smtk::mesh::ManagerPtr manager = this->m_manager.lock();

  //make sure we have a valid poly data
  if(!polydata)
    {
    return nullCollectionPtr;
    }
  else if( polydata->GetNumberOfPoints() == 0 ||
           polydata->GetNumberOfCells() == 0)
    {
    //early terminate if the polydata is empty.
    return nullCollectionPtr;
    }
  else if(!manager)
    {
    return nullCollectionPtr;
    }

  bool pointsConverted = false;
  bool cellsConverted = false;
  bool meshCreated = false;

  //Step 1
  //create the collection the polydata will be added to
  smtk::mesh::CollectionPtr collection = manager->makeCollection();

  //Step 2 allocate space for coordinates and load them into moab
  smtk::mesh::InterfacePtr iface = collection->interface();
  smtk::mesh::AllocatorPtr ialloc = iface->allocator();

  smtk::mesh::Handle firstVertHandle;
  pointsConverted = detail::convertVTKPoints(polydata->GetPoints(),
                                             ialloc,
                                             firstVertHandle);


  //step 3 allocate and fill connectivity
  smtk::mesh::HandleRange newlyCreatedCells;
  cellsConverted = detail::convertVTKCells( polydata,
                                            ialloc,
                                            firstVertHandle,
                                            newlyCreatedCells );

  if(pointsConverted && cellsConverted)
    {
    if(materialPropertyName.empty())
      { //if we don't have a material we create a single mesh
      smtk::mesh::Handle vtkMeshHandle;
      meshCreated = iface->createMesh( newlyCreatedCells, vtkMeshHandle);
      }
    else
      { //make multiple meshes each one assigned a material value
      meshCreated = detail::convertDomain(polydata->GetCellData(),
                                            iface,
                                            newlyCreatedCells,
                                            materialPropertyName);
      }
    }

  if(meshCreated)
    {
    return collection;
    }
  else
    {
    //failed to convert the data. this should be the only
    //place we return nullCollectionPtr after collection was created
    manager->removeCollection( collection );
    return nullCollectionPtr;
    }
}

//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr
VTKDataConverter::operator()(vtkUnstructuredGrid* ugrid,
                             std::string materialPropertyName) const
{
  smtk::mesh::CollectionPtr nullCollectionPtr;
  smtk::mesh::ManagerPtr manager = this->m_manager.lock();

  //make sure we have a valid poly data
  if(!ugrid)
    {
    return nullCollectionPtr;
    }
  else if( ugrid->GetNumberOfPoints() == 0 ||
           ugrid->GetNumberOfCells() == 0)
    {
    //early terminate if the ugrid is empty.
    return nullCollectionPtr;
    }
  else if(!manager)
    {
    return nullCollectionPtr;
    }

  bool pointsConverted = false;
  bool cellsConverted = false;
  bool meshCreated = false;

  //Step 1
  //create the collection the ugrid will be added to
  smtk::mesh::CollectionPtr collection = manager->makeCollection();

  //Step 2 allocate space for coordinates and load them into moab
  smtk::mesh::InterfacePtr iface = collection->interface();
  smtk::mesh::AllocatorPtr ialloc = iface->allocator();

  smtk::mesh::Handle firstVertHandle;
  pointsConverted = detail::convertVTKPoints(ugrid->GetPoints(),
                                               ialloc,
                                               firstVertHandle);

  //step 3 allocate and fill connectivity
  smtk::mesh::HandleRange newlyCreatedCells;
  cellsConverted = detail::convertVTKCells( ugrid,
                                            ialloc,
                                            firstVertHandle,
                                            newlyCreatedCells );

  if(pointsConverted && cellsConverted)
    {
    if(materialPropertyName.empty())
      { //if we don't have a material we create a single mesh
      smtk::mesh::Handle vtkMeshHandle;
      meshCreated = iface->createMesh( newlyCreatedCells, vtkMeshHandle);
      }
    else
      { //make multiple meshes each one assigned a material value
      meshCreated = detail::convertDomain(ugrid->GetCellData(),
                                            iface,
                                            newlyCreatedCells,
                                            materialPropertyName);
      }
    }

  if(meshCreated)
    {
    return collection;
    }
  else
    {
    //failed to convert the data. this should be the only
    //place we return nullCollectionPtr after collection was created
    manager->removeCollection( collection );
    return nullCollectionPtr;
    }
}

}
}
}
