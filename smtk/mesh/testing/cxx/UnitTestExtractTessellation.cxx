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

#include "smtk/io/ImportMesh.h"

#include "smtk/mesh/testing/cxx/helpers.h"

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
    test( static_cast<const std::size_t>(this->m_conn[offset+ i]) ==
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
smtk::mesh::CollectionPtr load_mesh(smtk::mesh::ManagerPtr mngr)
{
  std::string file_path(data_root);
  file_path += "/mesh/sixth_hexflatcore.h5m";

  smtk::mesh::CollectionPtr c  = smtk::io::ImportMesh::entireFile(file_path, mngr);
  test( c->isValid(), "collection should be valid");

  return c;
}

//----------------------------------------------------------------------------
void verify_constructors(const smtk::mesh::CollectionPtr& c)
{

  smtk::mesh::MeshSet ms = c->meshes( );

  //construct tessellation object that only wants connectivity info
  {
    std::vector<boost::int64_t> conn( 1 ); //size doesn't matter right now

    smtk::mesh::PreAllocatedTessellation tess(&conn[0]);

    test(tess.hasConnectivity() == true);
    test(tess.hasCellLocations() == false);
    test(tess.hasCellTypes() == false);
    test(tess.hasDoublePoints()  == false);
    test(tess.hasFloatPoints() == false);

    test(tess.useVTKConnectivity() == true);
    tess.disableVTKStyleConnectivity(true);
    test(tess.useVTKConnectivity() == false);

    test(tess.useVTKCellTypes() == true);
    tess.disableVTKCellTypes(true);
    test(tess.useVTKCellTypes() == false);

  }

  //construct tessellation object that only wants connectivity info and points
  {
    std::vector<boost::int64_t> conn( 1 ); //size doesn't matter right now
    std::vector<float> fpoints(1);   //size doesn't matter right now

    smtk::mesh::PreAllocatedTessellation ftess(&conn[0], &fpoints[0]);

    test(ftess.hasConnectivity() == true);
    test(ftess.hasCellLocations() == false);
    test(ftess.hasCellTypes() == false);
    test(ftess.hasDoublePoints()  == false);
    test(ftess.hasFloatPoints()  == true);
    test(ftess.useVTKConnectivity() == true);
    test(ftess.useVTKCellTypes() == true);

    //now test with doubles
    std::vector<double> dpoints(1);   //size doesn't matter right now
    smtk::mesh::PreAllocatedTessellation dtess(&conn[0], &dpoints[0]);

    test(dtess.hasConnectivity() == true);
    test(dtess.hasCellLocations() == false);
    test(dtess.hasCellTypes() == false);
    test(dtess.hasDoublePoints()  == true);
    test(dtess.hasFloatPoints() == false);
    test(dtess.useVTKConnectivity() == true);
    test(dtess.useVTKCellTypes() == true);
  }

  //construct tessellation object that only wants connectivity info, cell types
  //and cell locations bust doesnt want points
  {
    std::vector<boost::int64_t> conn( 1 ); //size doesn't matter right now
    std::vector<boost::int64_t> locations( 1 ); //size doesn't matter right now
    std::vector<unsigned char> types( 1 ); //size doesn't matter right now

    smtk::mesh::PreAllocatedTessellation tess(&conn[0],
                                              &locations[0],
                                              &types[0]);

    test(tess.hasConnectivity() == true);
    test(tess.hasCellLocations() == true);
    test(tess.hasCellTypes() == true);
    test(tess.hasDoublePoints()  == false);
    test(tess.hasFloatPoints() == false);
    test(tess.useVTKConnectivity() == true);
    test(tess.useVTKCellTypes() == true);
  }

  //construct tessellation object that wants everything
  {
    std::vector<boost::int64_t> conn( 1 ); //size doesn't matter right now
    std::vector<boost::int64_t> locations( 1 ); //size doesn't matter right now
    std::vector<unsigned char> types( 1 ); //size doesn't matter right now
    std::vector<float> fpoints(1);   //size doesn't matter right now

    smtk::mesh::PreAllocatedTessellation ftess(&conn[0],
                                               &locations[0],
                                               &types[0],
                                               &fpoints[0]);

    test(ftess.hasConnectivity() == true);
    test(ftess.hasCellLocations() == true);
    test(ftess.hasCellTypes() == true);
    test(ftess.hasDoublePoints()  == false);
    test(ftess.hasFloatPoints()  == true);
    test(ftess.useVTKConnectivity() == true);
    test(ftess.useVTKCellTypes() == true);

    //now test with doubles
    std::vector<double> dpoints(1);   //size doesn't matter right now
    smtk::mesh::PreAllocatedTessellation dtess(&conn[0],
                                               &locations[0],
                                               &types[0],
                                               &dpoints[0]);

    test(dtess.hasConnectivity() == true);
    test(dtess.hasCellLocations() == true);
    test(dtess.hasCellTypes() == true);
    test(dtess.hasDoublePoints()  == true);
    test(dtess.hasFloatPoints() == false);
    test(dtess.useVTKConnectivity() == true);
    test(dtess.useVTKCellTypes() == true);
  }
}

//----------------------------------------------------------------------------
void verify_alloc_lengths_meshset(const smtk::mesh::CollectionPtr& c)
{

  smtk::mesh::MeshSet all_meshes = c->meshes( );
  smtk::mesh::MeshSet mesh3d = c->meshes(  smtk::mesh::Dims3 );
  smtk::mesh::MeshSet mesh2d = c->meshes(  smtk::mesh::Dims2 );


  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(all_meshes,
                                                                   connectivityLength,
                                                                   numberOfCells,
                                                                   numberOfPoints);


  test(connectivityLength != -1);
  test(numberOfCells != -1);
  test(numberOfPoints != -1);

  test(static_cast<unsigned long>(connectivityLength) ==
       all_meshes.pointConnectivity().size() );
  test(static_cast<unsigned long>(numberOfCells) == all_meshes.cells().size() );
  test(static_cast<unsigned long>(numberOfPoints) ==
       all_meshes.points().size() );


  //Now try asking only for 3d cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(mesh3d,
                                                                   connectivityLength,
                                                                   numberOfCells,
                                                                   numberOfPoints);

   test(static_cast<unsigned long>(connectivityLength) ==
        mesh3d.pointConnectivity().size() );
   test(static_cast<unsigned long>(numberOfCells) == mesh3d.cells().size() );
   test(static_cast<unsigned long>(numberOfPoints) == mesh3d.points().size() );

  //Now try asking only for 2d cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(mesh2d,
                                                                   connectivityLength,
                                                                   numberOfCells,
                                                                   numberOfPoints);

  test(static_cast<unsigned long>(connectivityLength)
       == mesh2d.pointConnectivity().size() );
  test(static_cast<unsigned long>(numberOfCells) == mesh2d.cells().size() );
  test(static_cast<unsigned long>(numberOfPoints) == mesh2d.points().size() );

}

//----------------------------------------------------------------------------
void verify_alloc_lengths_cellset(const smtk::mesh::CollectionPtr& c)
{

  smtk::mesh::CellSet all_cells = c->cells( );
  smtk::mesh::CellSet cells3d = c->cells(  smtk::mesh::Dims3 );
  smtk::mesh::CellSet cells2d = c->cells(  smtk::mesh::Dims2 );


  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(all_cells,
                                                                   connectivityLength,
                                                                   numberOfCells,
                                                                   numberOfPoints);


  test(connectivityLength != -1);
  test(numberOfCells != -1);
  test(numberOfPoints != -1);

  test(static_cast<unsigned long>(connectivityLength) ==
       all_cells.pointConnectivity().size() );
  test(static_cast<unsigned long>(numberOfCells) == all_cells.size() );
  test(static_cast<unsigned long>(numberOfPoints)
       == all_cells.points().size() );


  //Now try asking only for 3d cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(cells3d,
                                                                   connectivityLength,
                                                                   numberOfCells,
                                                                   numberOfPoints);

  test(static_cast<unsigned long>(connectivityLength) ==
       cells3d.pointConnectivity().size() );
  test(static_cast<unsigned long>(numberOfCells) ==
       cells3d.size() );
  test(static_cast<unsigned long>(numberOfPoints) == cells3d.points().size() );

  //Now try asking only for 2d cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(cells2d,
                                                                   connectivityLength,
                                                                   numberOfCells,
                                                                   numberOfPoints);

  test(static_cast<unsigned long>(connectivityLength)
       == cells2d.pointConnectivity().size() );
  test(static_cast<unsigned long>(numberOfCells) == cells2d.size() );
  test(static_cast<unsigned long>(numberOfPoints) == cells2d.points().size() );

}


//----------------------------------------------------------------------------
void verify_extract_packed_single_type(const smtk::mesh::CollectionPtr& c)
{

  smtk::mesh::MeshSet all_meshes = c->meshes( );
  smtk::mesh::CellSet quads = c->cells( smtk::mesh::Quad );

  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(quads,
                                                                   connectivityLength,
                                                                   numberOfCells,
                                                                   numberOfPoints);

  std::vector<boost::int64_t> conn( connectivityLength );
  std::vector<float> fpoints(numberOfPoints * 3);

  smtk::mesh::PreAllocatedTessellation ftess(&conn[0], &fpoints[0]);

  ftess.disableVTKStyleConnectivity(true);
  ftess.disableVTKCellTypes(true);
  smtk::mesh::extractTessellation(quads, ftess);

  //lets iterate the points and make sure they all match
  VerifyPoints<float> vp(fpoints);
  smtk::mesh::for_each(quads.points(), vp);
}

//----------------------------------------------------------------------------
void verify_extract_only_connectivity_and_types(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::CellSet cells3d = c->cells( smtk::mesh::Dims3 );

  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(cells3d,
                                                                   connectivityLength,
                                                                   numberOfCells,
                                                                   numberOfPoints);

  std::vector<boost::int64_t> conn( connectivityLength );
  std::vector<boost::int64_t> locations( numberOfCells );
  std::vector<unsigned char> types( numberOfCells );

  smtk::mesh::PreAllocatedTessellation tess(&conn[0],&locations[0],&types[0]);

  tess.disableVTKStyleConnectivity(true);
  tess.disableVTKCellTypes(true);
  smtk::mesh::extractTessellation(cells3d, tess);

  //lets iterate the cells, and verify that the extraction matches
  //what we see when we iterate
  VerifyCells vc(cells3d, conn, locations, types, false);
  smtk::mesh::for_each(cells3d, vc);
  test( vc.cells(c) == cells3d);
}

//----------------------------------------------------------------------------
void verify_extract_all_to_vtk(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::CellSet cells3d = c->cells( smtk::mesh::Dims3 );

  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(cells3d,
                                                                   connectivityLength,
                                                                   numberOfCells,
                                                                   numberOfPoints);

  std::vector<boost::int64_t> conn( connectivityLength + numberOfCells );
  std::vector<boost::int64_t> locations( numberOfCells );
  std::vector<unsigned char> types( numberOfCells );
  std::vector<double> dpoints( numberOfPoints * 3 );

  smtk::mesh::PreAllocatedTessellation tess(&conn[0],
                                            &locations[0],
                                            &types[0],
                                            &dpoints[0]);

  smtk::mesh::extractTessellation(cells3d, tess);

  //lets iterate the cells, and verify that the extraction matches
  //what we see when we iterate
  VerifyCells vc(cells3d, conn, locations, types, true);
  smtk::mesh::for_each(cells3d, vc);
  test( vc.cells(c) == cells3d);

  //lets iterate the points and make sure they all match
  VerifyPoints<double> vp(dpoints);
  smtk::mesh::for_each(cells3d.points(), vp);
}

//----------------------------------------------------------------------------
void verify_extract_only_connectivity_to_vtk(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::CellSet cells2d = c->cells( smtk::mesh::Dims2 );

  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(cells2d,
                                                                   connectivityLength,
                                                                   numberOfCells,
                                                                   numberOfPoints);

  std::vector<boost::int64_t> conn( connectivityLength + numberOfCells );
  std::vector<boost::int64_t> locations( numberOfCells );
  std::vector<unsigned char> types( numberOfCells );

  smtk::mesh::PreAllocatedTessellation tess(&conn[0],
                                            &locations[0],
                                            &types[0]);

  smtk::mesh::extractTessellation(cells2d, tess);

  //lets iterate the cells, and verify that the extraction matches
  //what we see when we iterate
  VerifyCells vc(cells2d, conn, locations, types, true);
  smtk::mesh::for_each(cells2d, vc);
  test( vc.cells(c) == cells2d);
}

//----------------------------------------------------------------------------
void verify_extract_volume_meshes_by_global_points_to_vtk(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::CellSet cells = c->cells( smtk::mesh::Dims1 );
  cells.append( c->cells( smtk::mesh::Dims2 ) );

  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::PreAllocatedTessellation::determineAllocationLengths(cells,
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
  smtk::mesh::extractTessellation(cells, c->points(), tess);

  // //lets iterate the cells, and verify that the extraction matches
  // //what we see when we iterate
  VerifyCells vc(c->cells(), conn, locations, types, true);
  smtk::mesh::for_each(cells, vc);
  test( vc.cells(c) == cells);
}

}

//----------------------------------------------------------------------------
int UnitTestExtractTessellation(int, char** const)
{
  smtk::mesh::ManagerPtr mngr = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = load_mesh(mngr);

  verify_constructors(c);

  verify_alloc_lengths_meshset(c);
  verify_alloc_lengths_cellset(c);

  verify_extract_packed_single_type(c);
  verify_extract_only_connectivity_and_types(c);

  verify_extract_all_to_vtk(c);
  verify_extract_only_connectivity_to_vtk(c);

  // verify_extract_volume_meshes_by_global_points_to_vtk(c);

  return 0;
}
