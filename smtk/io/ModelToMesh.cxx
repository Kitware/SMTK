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

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Interface.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Volume.h"

#include <algorithm>

namespace smtk
{
namespace io
{

namespace detail
{

smtk::mesh::CellType tessToSMTKCell(smtk::model::Tessellation::size_type cell_shape)
{
  smtk::mesh::CellType ctype = smtk::mesh::CellType_MAX;
  switch (cell_shape)
  {
    case smtk::model::TESS_VERTEX:
      ctype = smtk::mesh::Vertex;
      break;
    case smtk::model::TESS_TRIANGLE:
      ctype = smtk::mesh::Triangle;
      break;
    case smtk::model::TESS_QUAD:
      ctype = smtk::mesh::Quad;
      break;
    case smtk::model::TESS_POLYGON:
      ctype = smtk::mesh::Polygon;
      break;
    case smtk::model::TESS_POLYLINE:
      ctype = smtk::mesh::Line;
      break;
    case smtk::model::TESS_POLYVERTEX:
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
  std::vector<smtk::model::EntityRef> withoutTess;
  for (it.begin(); !it.isAtEnd(); ++it)
  {
    if (!it->hasTessellation())
    {
      withoutTess.push_back(it.current());
    }
  }

  typedef std::vector<smtk::model::EntityRef>::const_iterator c_it;
  for (c_it i = withoutTess.begin(); i < withoutTess.end(); ++i)
  {
    ents.erase(*i);
  }
}

template <typename MappingType>
bool convert_vertices(
  const smtk::model::EntityRefs& ents, MappingType& mapping, const smtk::mesh::AllocatorPtr& ialloc)
{
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

    //All tessellations are stored with x,y,z coordinates.
    numPointsToAlloc += (modelCoords.size() / 3);
  }

  std::vector<double*> meshCoords;
  smtk::mesh::Handle firstVertHandle;
  const bool pointsAllocated =
    ialloc->allocatePoints(numPointsToAlloc, firstVertHandle, meshCoords);
  if (!pointsAllocated)
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

    const std::size_t length = modelCoords.size();

    //mark for this entity where in the global pool of points we can
    //find the start of its coordinates, this will allow us to properly
    //create the new connectivity for the mesh referencing the global
    //point coordinate array
    mapping[*it] = firstVertHandle + pos;

    for (std::size_t i = 0; i < length; i += 3, pos++)
    {
      meshCoords[0][pos] = modelCoords[i];
      meshCoords[1][pos] = modelCoords[i + 1];
      meshCoords[2][pos] = modelCoords[i + 2];
    }
  }

  //return true if we have copied each coordinate from the tessellation
  //into the mesh database.
  return (pos == numPointsToAlloc);
}

template <typename HandleData>
void convert_fixed_size_cell(std::vector<int>& cell_conn, smtk::mesh::CellType cellType,
  smtk::model::Tessellation::size_type numVerts, std::vector<std::size_t>& numCellsOfType,
  std::vector<HandleData>& cellMBConn, std::size_t global_coordinate_offset)
{
  std::size_t idx = numCellsOfType[cellType]++;

  smtk::mesh::Handle* currentConnLoc = cellMBConn[cellType].second + numVerts * idx;
  for (int j = 0; j < numVerts; ++j)
  {
    //we need to fix the cell_conn to account for the fact that the
    //values contained within are all relative to the tessellation and
    //we need location based on the total sum of all tessellations.
    //local position + ( total connectivity added before the tess ) should
    //be correct.
    currentConnLoc[j] = global_coordinate_offset + cell_conn[j];
  }
}

template <typename HandleData>
void convert_vertex(std::vector<int>&, smtk::mesh::CellType cellType,
  smtk::model::Tessellation::size_type numVerts, std::vector<std::size_t>& numCellsOfType,
  std::vector<HandleData>& cellMBConn, std::size_t global_coordinate_offset)
{
  numCellsOfType[cellType]++;

  //get the list of vertex cells for this entity
  smtk::mesh::HandleRange& currentCellids = cellMBConn[cellType].first;

  //instead of changing the connectivity like a normal cell, instead
  //we modify the cell id list, as it is empty since no cells are allocated
  //for vertices
  currentCellids.insert(global_coordinate_offset, //insert is inclusive on both ends
    global_coordinate_offset + numVerts - 1);
}

template <typename HandleData>
void convert_poly_line(std::vector<int>& cell_conn, smtk::mesh::CellType cellType,
  smtk::model::Tessellation::size_type numCellsInPolyCell, std::vector<std::size_t>& numCellsOfType,
  std::vector<HandleData>& cellMBConn, std::size_t global_coordinate_offset)
{
  //numVerts represents the number of verts in the tessellation, not
  //the number of verts in the mesh cell
  int previous_id = static_cast<int>(numCellsOfType[cellType]);
  numCellsOfType[cellType] += numCellsInPolyCell;

  int numVertsPerCell = smtk::mesh::verticesPerCell(cellType);
  smtk::mesh::Handle* currentConnLoc =
    cellMBConn[cellType].second + (numVertsPerCell * previous_id);

  int i = 0;
  for (int j = 0; j < numCellsInPolyCell; ++j)
  {
    currentConnLoc[i] = global_coordinate_offset + cell_conn[j];
    currentConnLoc[i + 1] = global_coordinate_offset + cell_conn[j + 1];
    i += 2;
  }
}

template <typename MappingType>
std::map<smtk::model::EntityRef, smtk::mesh::HandleRange> convert_cells(
  const smtk::model::EntityRefs& ents, MappingType& mapping, const smtk::mesh::AllocatorPtr& ialloc)
{
  typedef smtk::model::Tessellation Tess;

  std::map<smtk::model::EntityRef, smtk::mesh::HandleRange> newlyCreatedCells;

  smtk::model::EntityIterator it;
  it.traverse(ents.begin(), ents.end(), smtk::model::ITERATE_BARE);
  for (it.begin(); !it.isAtEnd(); ++it)
  {
    smtk::mesh::HandleRange cellsForThisEntity;
    const smtk::model::EntityRef& refForThisEntity = it.current();

    //we filtered out all ents without tess already, so this can't be null
    const Tess* tess = it->hasTessellation();

    // Convert the connectivity of this volume tessellation to a meshset.
    // We make 2 passes through the tessellation: one to determine the
    // allocation and another to create the cells. Note that each cell
    // type must be put in a separate handle range.
    //
    // TODO: This does not handle triangle strips/fans, or polygons
    Tess::size_type start_off;
    std::vector<std::size_t> numCellsOfType(smtk::mesh::CellType_MAX, 0);
    for (start_off = tess->begin(); start_off != tess->end();
         start_off = tess->nextCellOffset(start_off))
    //from end means that we properly handle runs of the same cell type
    {
      Tess::size_type cell_type;
      Tess::size_type numVertsPerCell = tess->numberOfCellVertices(start_off, &cell_type);
      Tess::size_type cell_shape = tess->cellShapeFromType(cell_type);
      if (cell_shape == smtk::model::TESS_VERTEX || cell_shape == smtk::model::TESS_TRIANGLE ||
        cell_shape == smtk::model::TESS_QUAD)
      {
        numCellsOfType[tessToSMTKCell(cell_shape)]++;
      }
      else if (cell_shape == smtk::model::TESS_POLYLINE)
      {
        //In a polyline the number of cells is equal to one less than the
        //number of points
        numCellsOfType[tessToSMTKCell(cell_shape)] += numVertsPerCell - 1;
      }
    }

    // Allocate handles.
    typedef std::pair<smtk::mesh::HandleRange, smtk::mesh::Handle*> HandleData;
    HandleData blank;
    blank.second = NULL;
    std::vector<HandleData> cellMBConn(smtk::mesh::CellType_MAX, blank);
    std::vector<HandleData>::iterator allocIt = cellMBConn.begin();
    std::vector<std::size_t>::iterator cellsOfTypeIt = numCellsOfType.begin();
    for (std::size_t ctype = 0; ctype != smtk::mesh::CellType_MAX;
         ++ctype, ++cellsOfTypeIt, ++allocIt)
    {
      if (*cellsOfTypeIt <= 0)
        continue;

      smtk::mesh::CellType cellType = static_cast<smtk::mesh::CellType>(ctype);
      int numVertsPerCell = smtk::mesh::verticesPerCell(cellType);
      if (cellType == smtk::mesh::Vertex)
      {
        //In the moab/interface world vertices don't have explicit connectivity
        //so we can't allocate cells. Instead we just explicitly add those
        //points to the MeshSet
        continue;
      }

      if (!ialloc->allocateCells(
            cellType, *cellsOfTypeIt, numVertsPerCell, allocIt->first, allocIt->second))
      { // error
        std::cerr << "Could not allocate cells\n";
      }
    }

    std::vector<int> cell_conn;
    numCellsOfType = std::vector<std::size_t>(smtk::mesh::CellType_MAX, 0);
    for (start_off = tess->begin(); start_off != tess->end();
         start_off = tess->nextCellOffset(start_off))
    {
      //fetch the number of cell vertices, and the cell type in a single query
      Tess::size_type cell_type;
      Tess::size_type numVerts = tess->numberOfCellVertices(start_off, &cell_type);
      Tess::size_type cell_shape = tess->cellShapeFromType(cell_type);

      //convert from tess type to smtk cell type
      const smtk::mesh::CellType cellType = tessToSMTKCell(cell_shape);
      std::size_t global_coordinate_offset = mapping[*it];

      cell_conn.reserve(numVerts);
      tess->vertexIdsOfCell(start_off, cell_conn);
      if (cell_shape == smtk::model::TESS_TRIANGLE || cell_shape == smtk::model::TESS_QUAD)
      {
        convert_fixed_size_cell(
          cell_conn, cellType, numVerts, numCellsOfType, cellMBConn, global_coordinate_offset);
      }
      else if (cell_shape == smtk::model::TESS_VERTEX)
      {
        convert_vertex(
          cell_conn, cellType, numVerts, numCellsOfType, cellMBConn, global_coordinate_offset);
      }
      else if (cell_shape == smtk::model::TESS_POLYLINE)
      {
        convert_poly_line(cell_conn, cellType, (numVerts - 1), numCellsOfType, cellMBConn,
          global_coordinate_offset);
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
    for (std::size_t ctype = 0; ctype != smtk::mesh::CellType_MAX;
         ++ctype, ++cellsOfTypeIt, ++allocIt)
    {
      if (*cellsOfTypeIt <= 0)
        continue;

      //update the connectivity as long as we aren't vertices
      smtk::mesh::CellType cellType = static_cast<smtk::mesh::CellType>(ctype);
      smtk::mesh::HandleRange& currentCellids = allocIt->first;
      if (cellType != smtk::mesh::Vertex)
      {
        int numVertsPerCell = smtk::mesh::verticesPerCell(cellType);
        smtk::mesh::Handle* connectivity = allocIt->second;
        ialloc->connectivityModified(currentCellids, numVertsPerCell, connectivity);
      }

      //we need to add these cells to the range that represents all
      //cells for this volume
      cellsForThisEntity.insert(currentCellids.begin(), currentCellids.end());
    }

    //save all the cells of this volume
    newlyCreatedCells.insert(std::make_pair(refForThisEntity, cellsForThisEntity));
  }
  return newlyCreatedCells;
}

/// Recursively find all the entities with tessellation
void find_entities_with_tessellation(const smtk::model::EntityRef& root,
  smtk::model::EntityRefs& tessEntities, smtk::model::EntityRefs& touched)
{
  typedef smtk::model::EntityRefArray EntityRefArray;
  EntityRefArray children = (root.isModel()
      ? root.as<smtk::model::Model>().cellsAs<EntityRefArray>()
      : (root.isCellEntity()
            ? root.as<smtk::model::CellEntity>().boundingCellsAs<EntityRefArray>()
            : (root.isGroup() ? root.as<smtk::model::Group>().members<EntityRefArray>()
                              : EntityRefArray())));

  if (root.isModel())
  {
    // Make sure sub-models and groups are handled.
    EntityRefArray tmp;
    tmp = root.as<smtk::model::Model>().submodelsAs<EntityRefArray>();
    children.insert(children.end(), tmp.begin(), tmp.end());
    tmp = root.as<smtk::model::Model>().groupsAs<EntityRefArray>();
    children.insert(children.end(), tmp.begin(), tmp.end());
  }

  for (EntityRefArray::const_iterator it = children.begin(); it != children.end(); ++it)
  {
    if (touched.find(*it) == touched.end())
    {
      touched.insert(*it);
      if (it->hasTessellation())
      {
        tessEntities.insert(*it);
      }
      detail::find_entities_with_tessellation(*it, tessEntities, touched);
    }
  }
}
} //namespace detail

ModelToMesh::ModelToMesh()
  : m_mergeDuplicates(true)
  , m_tolerance(-1)
{
}

smtk::mesh::CollectionPtr ModelToMesh::operator()(
  const smtk::mesh::ManagerPtr& meshManager, const smtk::model::ManagerPtr& modelManager) const
{
  typedef smtk::model::EntityRefs EntityRefs;
  typedef smtk::model::EntityTypeBits EntityTypeBits;
  typedef std::map<smtk::model::EntityRef, std::size_t> CoordinateOffsetMap;

  smtk::mesh::CollectionPtr nullCollectionPtr;
  if (!meshManager || !modelManager)
  {
    return nullCollectionPtr;
  }

  if (modelManager->tessellations().empty())
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

  EntityTypeBits etypes[4] = { smtk::model::VERTEX, smtk::model::EDGE, smtk::model::FACE,
    smtk::model::VOLUME };
  for (int i = 0; i != 4; ++i)
  {
    //extract all the coordinates from every tessellation and make a single
    //big pool
    EntityTypeBits entType = etypes[i];
    EntityRefs currentEnts = modelManager->entitiesMatchingFlagsAs<EntityRefs>(entType);
    detail::removeOnesWithoutTess(currentEnts);
    detail::convert_vertices(currentEnts, coordinateLocationMapping, ialloc);
  }

  //We need to iterate over each model i think here
  //next we convert all volumes, faces, edges, and vertices that have tessellation
  for (int i = 0; i != 4; ++i)
  {
    EntityTypeBits entType = etypes[i];
    EntityRefs currentEnts = modelManager->entitiesMatchingFlagsAs<EntityRefs>(entType);
    detail::removeOnesWithoutTess(currentEnts);
    if (!currentEnts.empty())
    {
      //for each volume entity we need to create a range of handles
      //that represent the cell ids for that volume.
      std::map<smtk::model::EntityRef, smtk::mesh::HandleRange> per_ent_cells =
        detail::convert_cells(currentEnts, coordinateLocationMapping, ialloc);

      typedef std::map<smtk::model::EntityRef, smtk::mesh::HandleRange>::const_iterator c_it;
      for (c_it j = per_ent_cells.begin(); j != per_ent_cells.end(); ++j)
      {
        //now create a mesh from those cells
        smtk::mesh::CellSet cellsForMesh(collection, j->second);
        smtk::mesh::MeshSet ms = collection->createMesh(cellsForMesh);
        collection->setAssociation(j->first, ms);
      }

      EntityRefs currentModels =
        modelManager->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::MODEL_ENTITY);
      if (currentModels.size() > 0)
      {
        collection->associateToModel(currentModels.begin()->entity());
      }
    }
  }

  //Now merge all duplicate points inside the collection
  if (this->m_mergeDuplicates)
  {
    if (this->m_tolerance >= 0)
    {
      collection->meshes().mergeCoincidentContactPoints(this->m_tolerance);
    }
    else
    { //allow the meshes api to specify the default
      collection->meshes().mergeCoincidentContactPoints();
    }
  }

  return collection;
}

smtk::mesh::CollectionPtr ModelToMesh::operator()(const smtk::model::Model& model) const
{
  typedef smtk::model::EntityRefs EntityRefs;
  typedef std::map<smtk::model::EntityRef, std::size_t> CoordinateOffsetMap;
  smtk::model::ManagerPtr modelManager = model.manager();
  smtk::mesh::CollectionPtr nullCollectionPtr;
  if (!modelManager || !modelManager->meshes())
  {
    return nullCollectionPtr;
  }

  if (modelManager->tessellations().empty())
  {
    //we have zero tesselations, we can't continue. This is an invalid model
    return nullCollectionPtr;
  }

  smtk::mesh::ManagerPtr meshManager = modelManager->meshes();
  //Create the collection and extract the allocation interface from it
  smtk::mesh::CollectionPtr collection = meshManager->makeCollection();
  smtk::mesh::InterfacePtr iface = collection->interface();
  smtk::mesh::AllocatorPtr ialloc = iface->allocator();
  collection->setModelManager(modelManager);

  //We create a new mesh each for the Edge(s), Face(s) and Volume(s).
  //the MODEL_ENTITY will be associated with the meshset that contains all
  // meshes.
  CoordinateOffsetMap coordinateLocationMapping;
  EntityRefs tessEntities, touched;
  detail::find_entities_with_tessellation(model, tessEntities, touched);
  if (!tessEntities.empty())
  {
    detail::convert_vertices(tessEntities, coordinateLocationMapping, ialloc);

    //for each volumes, faces, edges, and vertices entity we need to create a range of handles
    //that represent the cell ids for that volume.
    std::map<smtk::model::EntityRef, smtk::mesh::HandleRange> per_ent_cells =
      detail::convert_cells(tessEntities, coordinateLocationMapping, ialloc);

    typedef std::map<smtk::model::EntityRef, smtk::mesh::HandleRange>::const_iterator c_it;
    for (c_it i = per_ent_cells.begin(); i != per_ent_cells.end(); ++i)
    {
      //now create a mesh from those cells
      smtk::mesh::CellSet cellsForMesh(collection, i->second);
      smtk::mesh::MeshSet ms = collection->createMesh(cellsForMesh);
      collection->setAssociation(i->first, ms);
    }

    collection->associateToModel(model.entity());
  }

  //Now merge all duplicate points inside the collection
  if (this->m_mergeDuplicates)
  {
    if (this->m_tolerance >= 0)
    {
      collection->meshes().mergeCoincidentContactPoints(this->m_tolerance);
    }
    else
    { //allow the meshes api to specify the default
      collection->meshes().mergeCoincidentContactPoints();
    }
  }

  return collection;
}
}
}
