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

  std::vector<double *> meshCoords;
  smtk::mesh::Handle firstVertHandle;

  const bool pointsAllocated = ialloc->allocatePoints( ents.size(),
                                                       firstVertHandle,
                                                       meshCoords);
  if(!pointsAllocated)
    {
    return false;
    }

  //1. Convert each vertex into a mesh cell

  //track what cell we are creating
  smtk::mesh::Handle currentVertHandle = firstVertHandle;

  //iterate over the model verts
  smtk::model::EntityIterator it;
  it.traverse(ents.begin(), ents.end(), smtk::model::ITERATE_BARE);
  for (it.begin(); !it.isAtEnd(); ++it, ++currentVertHandle)
    {
    //we filtered out all ents without tess already, so this can't be null
    const smtk::model::Tessellation* tess = it->hasTessellation();

    //convert the 3d coords of the tesselation
    std::vector<double> const& modelCoords = tess->coords();
    meshCoords[0][0] = modelCoords[0];
    meshCoords[0][1] = modelCoords[1];
    meshCoords[0][2] = modelCoords[2];

    mapping.insert( value_type(it->entity(), currentVertHandle ) );
    }

  return true;

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

    //convert the connectivity of this volume tesselation to a meshset
    Tess::size_type start_off;
    Tess::size_type end_off;
    for (start_off = tess->begin();
         start_off != tess->end();
         start_off = tess->nextCellOffset(end_off)) //yes from end_off
         //from end means that we properly handle runs of the same cell type
      {
      std::size_t numCellsToAlloc = 1; //count the start cell

      //fetch the number of cell vertices, and the cell type in a single query
      Tess::size_type cell_type;
      Tess::size_type numVertsPerCell = tess->numberOfCellVertices(start_off, &cell_type);
      Tess::size_type cell_shape = tess->cellShapeFromType(cell_type);

      //convert from tess type to smtk cell type
      const smtk::mesh::CellType cellType = tessToSMTKCell( cell_shape );

      //the most efficient way to allocate is to do batch allocation, so lets
      //iterate all cells of the same shape. This can only be done for
      //points, triangles and quads, as everything else has variable cell length
      if(cell_shape == smtk::model::TESS_VERTEX   ||
         cell_shape == smtk::model::TESS_TRIANGLE ||
         cell_shape == smtk::model::TESS_QUAD)
        {
        for(end_off = start_off; end_off != tess->end(); end_off = tess->nextCellOffset(end_off))
          { numCellsToAlloc++; }
        }

      //need to convert from tess cell type to moab cell type
      bool allocated = false;
      smtk::mesh::Handle *startOfConnectivityArray = NULL;

      //only convert cells smtk mesh supports
      smtk::mesh::HandleRange cellsCreatedForThisType;
      allocated = ialloc->allocateCells( cellType,
                                         numCellsToAlloc,
                                         static_cast<int>(numVertsPerCell),
                                         cellsCreatedForThisType,
                                         startOfConnectivityArray);

      if(allocated)
        {
        //now that we have the chunk allocated, we have re-iterate and get
        //the connectivity for each cell
        smtk::mesh::Handle *currentConnLoc = startOfConnectivityArray;
        std::vector<int> cell_conn(numVertsPerCell);
        for(Tess::size_type i = start_off; i != end_off; i = tess->nextCellOffset(i))
          {
          tess->vertexIdsOfCell(i, cell_conn);
          for(std::size_t j=0; j < numVertsPerCell; ++j)
            {
            currentConnLoc[j] = cell_conn[j];
            }
          }

        // notify database that we have written to connectivity, that way
        // it can properly update adjacencies and other database info
        ialloc->connectivityModified(cellsCreatedForThisType,
                                     static_cast<int>(numVertsPerCell),
                                     startOfConnectivityArray);

        //we need to add these cells to the range that represents all
        //cells for this volume
        cellsForThisEntity.insert(cellsCreatedForThisType.begin(),
                                  cellsCreatedForThisType.end());
        }
      } //for all cells of a given type in a run

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
  typedef std::map< smtk::common::UUID, smtk::mesh::Handle > UUIDToMeshHandle;
  typedef UUIDToMeshHandle::const_iterator uuid_mesh_c_it;

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

  //We create a new mesh each for the Edge(s), Face(s) and Volume(s).
  //the MODEL_ENTITY will be associated with the meshset that contains all
  // meshes.

  //first thing we do is create a point mapping table so that we can
  //merge duplicate vertices
  UUIDToMeshHandle cellMapping;
  {
  EntityRefs justVerts = modelManager->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::VERTEX);
  //extract the verts that have tesselation
  detail::removeOnesWithoutTess(justVerts);
  detail::convert_vertices(justVerts, cellMapping, ialloc);
  }


  //We need to iterate over each model i think here
  //next we convert all volumes, faces, edges, and vertices that have tessellation
  EntityTypeBits entitiesToConvert[4] = { smtk::model::VERTEX,
                                          smtk::model::EDGE,
                                          smtk::model::FACE,
                                          smtk::model::VOLUME };
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
        detail::convert_cells(currentEnts, cellMapping, ialloc);

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
