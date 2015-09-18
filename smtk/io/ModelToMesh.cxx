//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ModelToMesh.h"

#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Interface.h"
#include "smtk/mesh/Collection.h"

#include "smtk/model/EntityIterator.h"
#include "smtk/model/Model.h"
#include "smtk/model/Volume.h"
#include "smtk/model/Tessellation.h"

#include <algorithm>

namespace smtk{
namespace io {

namespace detail
{

smtk::mesh::CellType tessToSMTKCell(smtk::model::Tessellation::size_type cell_shape)
{
  smtk::mesh::CellType ctype = smtk::mesh::CellType_MAX;
  switch (cell_shape)
    {
    case smtk::model::TESS_VERTEX:          ctype = smtk::mesh::Vertex; break;
    case smtk::model::TESS_TRIANGLE:        ctype = smtk::mesh::Triangle; break;
    case smtk::model::TESS_QUAD:            ctype = smtk::mesh::Quad; break;
    case smtk::model::TESS_POLYGON:         ctype = smtk::mesh::Polygon; break;
    case smtk::model::TESS_POLYVERTEX:
    case smtk::model::TESS_POLYLINE:
    default:
      //Polyvertex and polyline are currently un supported by smtk mesh
      ctype = smtk::mesh::CellType_MAX;
      break;
    }
  return ctype;
}

void removeOnesWithoutTess(smtk::model::EntityRefs& ents)
{
  smtk::model::EntityIterator it;
  it.traverse(ents.begin(), ents.end(), smtk::model::ITERATE_BARE);
  std::vector< smtk::model::EntityRef > withoutTess;
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    if(!it->hasTessellation())
      {
      withoutTess.push_back(it.current());
      }
    }

  typedef std::vector< smtk::model::EntityRef >::const_iterator c_it;
  for(c_it i=withoutTess.begin(); i < withoutTess.end(); ++i)
    {
    ents.erase(*i);
    }

}

template<typename MappingType>
bool convert_vertices(const smtk::model::EntityRefs& ents,
                      MappingType& mapping,
                      const smtk::mesh::AllocatorPtr& ialloc)
{
  typedef typename MappingType::value_type value_type;

  //count the number of points in the tessellation so that we can properly
  //allocate a single pool large enough for all the points
  std::size_t numPointsToAlloc = 0;
  smtk::model::EntityIterator it;
  it.traverse(ents.begin(), ents.end(), smtk::model::ITERATE_BARE);
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    //we filtered out all ents without tess already, so this can't be null
    const smtk::model::Tessellation* tess = it->hasTessellation();
    std::vector<double> const& modelCoords = tess->coords();

    //each model has an embedding dimension which dictates the number of
    //coordinates per point. This allows us to convert 2d / 1d points
    //safely into 3d space
    const int dimension = it->embeddingDimension();
    numPointsToAlloc += ( modelCoords.size() / dimension);
    }

  std::vector<double *> meshCoords;
  smtk::mesh::Handle firstVertHandle;
  const bool pointsAllocated = ialloc->allocatePoints( numPointsToAlloc,
                                                       firstVertHandle,
                                                       meshCoords);
  if(!pointsAllocated)
    {
    return false;
    }

  //iterate over the tessellation populating the mesh coordinates
  std::size_t pos = 0;
  it.traverse(ents.begin(), ents.end(), smtk::model::ITERATE_BARE);
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    //we filtered out all ents without tess already, so this can't be null
    const smtk::model::Tessellation* tess = it->hasTessellation();
    std::vector<double> const& modelCoords = tess->coords();

    const int dimension = it->embeddingDimension();
    const std::size_t length = modelCoords.size();

    //mark for this entity where in the global pool of points we can
    //find the start of its coordinates, this will allow us to properly
    //create the new connectivity for the mesh referencing the global
    //point coordinate array
    mapping[*it]=pos;

    //while a little more complex, this way avoids branching or comparisons
    //against dimension while filling the memory
    if(dimension == 3)
      {
      for( std::size_t i=0; i < length; i+=3, pos++)
        {
        meshCoords[0][pos] = modelCoords[i];
        meshCoords[1][pos] = modelCoords[i+1];
        meshCoords[2][pos] = modelCoords[i+2];
        }
      }
    else if(dimension == 2)
      {
      for( std::size_t i=0; i < length; i+=2, pos++)
        {
        meshCoords[0][pos] = modelCoords[i];
        meshCoords[1][pos] = modelCoords[i+1];
        }
      std::fill( meshCoords[2], meshCoords[2] + numPointsToAlloc, double(0));
      }
    else if(dimension == 1)
      {
      for(std::size_t i=0; i < length; i++, pos++)
        {
        meshCoords[0][pos] = modelCoords[i];
        }
      std::fill( meshCoords[1], meshCoords[1] + numPointsToAlloc, double(0));
      std::fill( meshCoords[2], meshCoords[2] + numPointsToAlloc, double(0));
      }
    }

  //return true if we have copied each coordinate from the tessellation
  //into the mesh database.
  return (pos == numPointsToAlloc);

}

//----------------------------------------------------------------------------
template<typename MappingType>
std::map<smtk::model::EntityRef, smtk::mesh::HandleRange>
convert_cells(const smtk::model::EntityRefs& ents,
              MappingType& mapping,
              const smtk::mesh::AllocatorPtr& ialloc)
{
  typedef smtk::model::Tessellation Tess;
  typedef typename MappingType::value_type value_type;

  std::map<smtk::model::EntityRef, smtk::mesh::HandleRange> newlyCreatedCells;

  smtk::model::EntityIterator it;
  it.traverse(ents.begin(), ents.end(), smtk::model::ITERATE_BARE);
  for (it.begin(); !it.isAtEnd(); ++it)
    {
    smtk::mesh::HandleRange cellsForThisEntity;
    const smtk::model::EntityRef& refForThisEntity = it.current();

    //we filtered out all ents without tess already, so this can't be null
    const Tess* tess = it->hasTessellation();

    // Convert the connectivity of this volume tesselation to a meshset.
    // We make 2 passes through the tessellation: one to determine the
    // allocation and another to create the cells. Note that each cell
    // type must be put in a separate handle range.
    //
    // TODO: This does not handle triangle strips/fans or other cell types
    //       with a varying number of vertices per cell.
    Tess::size_type start_off;
    std::vector<std::size_t> numCellsOfType(smtk::mesh::CellType_MAX, 0);
    for (start_off = tess->begin();
         start_off != tess->end();
         start_off = tess->nextCellOffset(start_off))
         //from end means that we properly handle runs of the same cell type
      {
      Tess::size_type cell_type;
      tess->numberOfCellVertices(start_off, &cell_type);
      Tess::size_type cell_shape = tess->cellShapeFromType(cell_type);
      if(cell_shape == smtk::model::TESS_VERTEX   ||
         cell_shape == smtk::model::TESS_TRIANGLE ||
         cell_shape == smtk::model::TESS_QUAD)
        numCellsOfType[tessToSMTKCell(cell_shape)]++;
      }

    // Allocate handles.
    typedef std::pair<smtk::mesh::HandleRange, smtk::mesh::Handle*> HandleData;
    HandleData blank;
    blank.second = NULL;
    std::vector<HandleData> cellMBConn(smtk::mesh::CellType_MAX, blank);
    std::vector<HandleData>::iterator allocIt = cellMBConn.begin();
    std::vector<std::size_t>::iterator cellsOfTypeIt = numCellsOfType.begin();
    for (std::size_t ctype = 0; ctype != smtk::mesh::CellType_MAX; ++ctype, ++cellsOfTypeIt, ++allocIt)
      {
      if (*cellsOfTypeIt <= 0)
        continue;

      smtk::mesh::CellType cellType = static_cast<smtk::mesh::CellType>(ctype);
      int numVertsPerCell = smtk::mesh::verticesPerCell(cellType);

      if (
        !ialloc->allocateCells(
          cellType, *cellsOfTypeIt, numVertsPerCell,
          allocIt->first, allocIt->second))
        { // error
        std::cerr << "Could not allocate cells\n";
        }
      }

    std::vector<int> cell_conn;
    numCellsOfType = std::vector<std::size_t>(smtk::mesh::CellType_MAX, 0);
    for (start_off = tess->begin();
         start_off != tess->end();
         start_off = tess->nextCellOffset(start_off))
      {
      //fetch the number of cell vertices, and the cell type in a single query
      Tess::size_type cell_type;
      Tess::size_type numVertsPerCell = tess->numberOfCellVertices(start_off, &cell_type);
      Tess::size_type cell_shape = tess->cellShapeFromType(cell_type);

      //convert from tess type to smtk cell type
      const smtk::mesh::CellType cellType = tessToSMTKCell( cell_shape );

      //the most efficient way to allocate is to do batch allocation, so lets
      //iterate all cells of the same shape. This can only be done for
      //points, triangles and quads, as everything else has variable cell length
      if(cell_shape != smtk::model::TESS_VERTEX   &&
         cell_shape != smtk::model::TESS_TRIANGLE &&
         cell_shape != smtk::model::TESS_QUAD)
        {
        continue;
        }

      int idx = numCellsOfType[cellType]++;
      std::size_t global_coordinate_offset = mapping[*it];

      smtk::mesh::Handle* currentConnLoc = cellMBConn[cellType].second + numVertsPerCell * idx;
      cell_conn.reserve(numVertsPerCell);
      tess->vertexIdsOfCell(start_off, cell_conn);
      for (int j=0; j < numVertsPerCell; ++j)
        {
        //we need to fix the cell_conn to account for the fact that the
        //values contained within are all relative to the tessellation and
        //we need location based on the total sum of all tessellations.
        //local position + ( total connectivity added before the tess ) should
        //be correct.
        currentConnLoc[j] = global_coordinate_offset + cell_conn[j];
        }

      //this is horribly important. vertexIdsOfCell is implemented by using
      //insert() and end() which means that it appends, and if we don't clear
      //the vector we will append onto the end and have an vector that
      //holds 2 triangles of connectivity ( than 3,4,5, etc).
      //We also use clear, to reduce the number of memory allocation we require
      cell_conn.clear();
      }

    allocIt = cellMBConn.begin();
    cellsOfTypeIt = numCellsOfType.begin();
    for (std::size_t ctype = 0; ctype != smtk::mesh::CellType_MAX; ++ctype, ++cellsOfTypeIt, ++allocIt)
      {
      if (*cellsOfTypeIt <= 0)
        continue;

      smtk::mesh::CellType cellType = static_cast<smtk::mesh::CellType>(ctype);
      int numVertsPerCell = smtk::mesh::verticesPerCell(cellType);

      // notify database that we have written to connectivity, that way
      // it can properly update adjacencies and other database info
      ialloc->connectivityModified(allocIt->first,
                                   numVertsPerCell,
                                   allocIt->second);

      //we need to add these cells to the range that represents all
      //cells for this volume
      cellsForThisEntity.insert(allocIt->first.begin(), allocIt->first.end());
      }

    //save all the cells of this volume
    newlyCreatedCells.insert( std::make_pair(refForThisEntity,cellsForThisEntity) );
    }
  return newlyCreatedCells;
}

} //namespace detail


//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr ModelToMesh::operator()(const smtk::mesh::ManagerPtr& meshManager,
                                                  const smtk::model::ManagerPtr& modelManager) const
{
  typedef smtk::model::EntityRefs EntityRefs;
  typedef smtk::model::EntityTypeBits EntityTypeBits;
  typedef std::map< smtk::model::EntityRef, std::size_t > CoordinateOffsetMap;

  smtk::mesh::CollectionPtr nullCollectionPtr;
  if(!meshManager || !modelManager )
    {
    return nullCollectionPtr;
    }

  if(modelManager->tessellations().empty())
    {
    //we have zero tesselations, we can't continue. This is an invalid model
    return nullCollectionPtr;
    }

  //Create the collection and extract the allocation interface from it
  smtk::mesh::CollectionPtr collection = meshManager->makeCollection();
  smtk::mesh::InterfacePtr iface = collection->interface();
  smtk::mesh::AllocatorPtr ialloc = iface->allocator();
  collection->setModelManager(modelManager);

  //We create a new mesh each for the Edge(s), Face(s) and Volume(s).
  //the MODEL_ENTITY will be associated with the meshset that contains all
  // meshes.
  CoordinateOffsetMap coordinateLocationMapping;
  for( int entAsInt =0; entAsInt != 4; ++entAsInt)
    {
    //extract all the coordinates from every tessellation and make a single
    //big pool
    EntityTypeBits entType = static_cast<EntityTypeBits>(entAsInt);
    EntityRefs currentEnts = modelManager->entitiesMatchingFlagsAs<EntityRefs>(entType);
    detail::removeOnesWithoutTess(currentEnts);
    detail::convert_vertices(currentEnts,
                             coordinateLocationMapping,
                             ialloc);
    }


  //We need to iterate over each model i think here
  //next we convert all volumes, faces, edges, and vertices that have tessellation
  for( int entAsInt =0; entAsInt != 4; ++entAsInt)
  {
  EntityTypeBits entType = static_cast<EntityTypeBits>(entAsInt);
  EntityRefs currentEnts = modelManager->entitiesMatchingFlagsAs<EntityRefs>(entType);
  detail::removeOnesWithoutTess( currentEnts );
  if( !currentEnts.empty() )
    {
    //for each volume entity we need to create a range of handles
    //that represent the cell ids for that volume.
    std::map<smtk::model::EntityRef, smtk::mesh::HandleRange> per_ent_cells =
        detail::convert_cells(currentEnts,
                              coordinateLocationMapping,
                              ialloc);

    typedef std::map<smtk::model::EntityRef, smtk::mesh::HandleRange>::const_iterator c_it;
    for(c_it i= per_ent_cells.begin(); i != per_ent_cells.end(); ++i)
      {
      //now create a mesh from those cells
      smtk::mesh::CellSet cellsForMesh(collection, i->second);
      smtk::mesh::MeshSet ms = collection->createMesh(cellsForMesh);
      collection->addAssociation(i->first, ms);
      }
    // collection->addAssociation(ModelEntityRef, all_ms);
    }
  }


  return collection;

}

}
}
