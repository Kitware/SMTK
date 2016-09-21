//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/ExtractTessellation.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityRef.h"

#include "smtk/io/ModelToMesh.h"
#include "smtk/io/ImportJSON.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"

#include <fstream>

namespace
{

//----------------------------------------------------------------------------
class VerifyCells : public smtk::mesh::CellForEach
{
  smtk::mesh::HandleRange m_cells;
  smtk::mesh::PointSet m_points;
  const std::vector<boost::int64_t>& m_conn;
  const std::vector<boost::int64_t>& m_locations;
  const std::vector<unsigned char>& m_types;

  std::size_t m_currentIndex;
  boost::int64_t m_currentLocation;

  bool m_is_vtk;
public:
//--------------------------------------------------------------------------
VerifyCells( const smtk::mesh::CellSet& cells,
             const std::vector<boost::int64_t>& conn,
             const std::vector<boost::int64_t>& locations,
             const std::vector<unsigned char>& types,
             bool is_vtk_conn):
  smtk::mesh::CellForEach(),
  m_cells(),
  m_points(cells.points()),
  m_conn(conn),
  m_locations(locations),
  m_types(types),
  m_currentIndex(0),
  m_currentLocation(0),
  m_is_vtk(is_vtk_conn)
  {
  }

//--------------------------------------------------------------------------
void forCell(const smtk::mesh::Handle& cellId,
             smtk::mesh::CellType cellType,
             int numPts)
{
  this->m_cells.insert(cellId);
  //verify the offset is in the correct location
  boost::int64_t offset = this->m_locations[this->m_currentIndex];
  test(offset == this->m_currentLocation);
  if(m_is_vtk)
    {
    //the connectivity at offset should hold the number of points
    test( this->m_conn[offset] == numPts );
    this->m_currentLocation++;
    offset++;
    }
  else
    {
    //verify the types match when doing smtk types
    test(this->m_types[this->m_currentIndex] == static_cast<unsigned char>(cellType));
    }

  //verify the points ids are mapped properly
  for(int i=0; i < numPts; ++i)
    {
    test( static_cast<std::size_t>(this->m_conn[offset+ i]) ==
          this->m_points.find(this->pointIds()[i] ) );
    }

  this->m_currentIndex++;
  this->m_currentLocation += numPts;
}

//--------------------------------------------------------------------------
smtk::mesh::CellSet cells(smtk::mesh::CollectionPtr c) const
{
  return smtk::mesh::CellSet(c,m_cells);
}

};

//----------------------------------------------------------------------------
template<typename T>
class VerifyPoints : public smtk::mesh::PointForEach
{
  const std::vector<T>& m_points;
  std::size_t m_currentIndex;
public:
VerifyPoints( const std::vector<T>& points ):
  smtk::mesh::PointForEach(),
  m_points(points),
  m_currentIndex(0)
{
}
//--------------------------------------------------------------------------
void forPoints(const smtk::mesh::HandleRange& pointIds,
               std::vector<double>& xyz,
               bool& coordinatesModified)
{
  coordinatesModified = false; //we are not modifying the coords

  typedef smtk::mesh::HandleRange::const_iterator c_it;
  std::size_t offset = 0;
  for(c_it i = pointIds.begin(); i != pointIds.end(); ++i)
    {
    //iterate the range of coords / point ids
    test( this->m_points[m_currentIndex ]  == static_cast<T>( xyz[offset] ) );
    test( this->m_points[m_currentIndex+1] == static_cast<T>( xyz[offset+1] ) );
    test( this->m_points[m_currentIndex+2] == static_cast<T>( xyz[offset+2]) );
    this->m_currentIndex += 3;
    offset+=3;
    }
}

};

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;


//----------------------------------------------------------------------------
void create_simple_mesh_model( smtk::model::ManagerPtr mgr )
{
  std::string file_path(data_root);
  file_path += "/model/2d/smtk/test2D.json";

  std::ifstream file(file_path.c_str());

  std::string json(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));

  //we should load in the test2D.json file as an smtk to model
  smtk::io::ImportJSON::intoModelManager(json.c_str(), mgr);
  mgr->assignDefaultNames();

  file.close();
}

//----------------------------------------------------------------------------
void verify_alloc_lengths_entityref(const smtk::model::EntityRef& eRef,
                                    const smtk::mesh::CollectionPtr& c)
{

  smtk::mesh::MeshSet mesh = c->findAssociatedMeshes( eRef );

  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(eRef, c,
                                                                   connectivityLength,
                                                                   numberOfCells,
                                                                   numberOfPoints);


  test(connectivityLength != -1);
  test(numberOfCells != -1);
  test(numberOfPoints != -1);

  test(static_cast<std::size_t>(connectivityLength) ==
       mesh.pointConnectivity().size() );
  test(static_cast<std::size_t>(numberOfCells) == mesh.cells().size() );
  test(static_cast<std::size_t>(numberOfPoints) == mesh.points().size() );

}

//----------------------------------------------------------------------------
void verify_extract(const smtk::model::EntityRef& eRef,
                    const smtk::mesh::CollectionPtr& c)
{
  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(eRef, c,
                                                                   connectivityLength,
                                                                   numberOfCells,
                                                                   numberOfPoints);

  std::vector<boost::int64_t> conn( connectivityLength );
  std::vector<float> fpoints(numberOfPoints * 3);

  smtk::mesh::PreAllocatedTessellation ftess(&conn[0], &fpoints[0]);

  ftess.disableVTKStyleConnectivity(true);
  ftess.disableVTKCellTypes(true);
  smtk::mesh::extractTessellation(eRef, c, ftess);

  //lets iterate the points and make sure they all match
  smtk::mesh::CellSet cells = c->findAssociatedCells( eRef );
  VerifyPoints<float> vp(fpoints);
  smtk::mesh::for_each(cells.points(), vp);
}

//----------------------------------------------------------------------------
void verify_extract_volume_meshes_by_global_points_to_vtk(
  const smtk::model::EntityRef& eRef, const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::CellSet cells = c->findAssociatedCells( eRef );

  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(eRef, c,
                                                                   connectivityLength,
                                                                   numberOfCells,
                                                                   numberOfPoints);

  std::vector<boost::int64_t> conn( connectivityLength + numberOfCells );
  std::vector<boost::int64_t> locations( numberOfCells );
  std::vector<unsigned char> types( numberOfCells );

  smtk::mesh::PreAllocatedTessellation tess(&conn[0],
                                            &locations[0],
                                            &types[0]);

  //extract in releation to the points of all the meshes
  smtk::mesh::extractTessellation(eRef, c, c->points(), tess);

  // //lets iterate the cells, and verify that the extraction matches
  // //what we see when we iterate
  VerifyCells vc(c->cells(), conn, locations, types, true);
  smtk::mesh::for_each(cells, vc);
  test( vc.cells(c) == cells);
}

//----------------------------------------------------------------------------
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

}

//----------------------------------------------------------------------------
int UnitTestExtractTessellationOfModel(int, char** const)
{
  // Somehow grab an EntityRef with an associated tessellation
  smtk::model::EntityRef eRef;
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_mesh_model(modelManager);

  smtk::io::ModelToMesh convert;
  convert.setIsMerging(false);
  smtk::mesh::CollectionPtr c = convert(meshManager,modelManager);

  typedef smtk::model::EntityRefs EntityRefs;
  typedef smtk::model::EntityTypeBits EntityTypeBits;

  EntityTypeBits etypes[4] = { smtk::model::VERTEX, smtk::model::EDGE,
                               smtk::model::FACE, smtk::model::VOLUME };
  for(int i=0; i != 4; ++i)
    {
    //extract all the coordinates from every tessellation and make a single
    //big pool
    EntityTypeBits entType = etypes[i];
    EntityRefs currentEnts = modelManager->entitiesMatchingFlagsAs<EntityRefs>(entType);
    removeOnesWithoutTess(currentEnts);
    if (!currentEnts.empty())
      {
      eRef = *currentEnts.begin();
      verify_alloc_lengths_entityref(eRef, c);
      verify_extract(eRef, c);
      verify_extract_volume_meshes_by_global_points_to_vtk(eRef, c);
      }
    }

  return 0;
}
