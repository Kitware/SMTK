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
#include "smtk/mesh/Manager.h"
#include "smtk/io/ImportMesh.h"

#include "smtk/mesh/testing/cxx/helpers.h"


namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;


//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr load_mesh(smtk::mesh::ManagerPtr mngr)
{
  std::string file_path(data_root);
  file_path += "/mesh/twoassm_out.h5m";

  smtk::mesh::CollectionPtr c  = smtk::io::ImportMesh::entireFile(file_path, mngr);
  test( c->isValid(), "collection should be valid");

  return c;
}

//----------------------------------------------------------------------------
void verify_constructors(const smtk::mesh::CollectionPtr& c)
{
  std::vector< std::string > mesh_names = c->meshNames();

  smtk::mesh::MeshSet ms = c->meshes( mesh_names[0] );
  smtk::mesh::PointSet ps = ms.points();

  smtk::mesh::PointSet ps2(ps);
  smtk::mesh::PointSet ps3 = c->meshes( "bad_name" ).points();
  test( ps3.is_empty() == true );
  test( ps3.size() == 0 );

  test( ps.size() == ps2.size());
  test( ps.size() != ps3.size());

  ps3 = ps; //test assignment operator
  test( ps.size() == ps3.size());

  test( ps.is_empty() == false );
  test( ps2.is_empty() == false );
  test( ps3.is_empty() == false );
}

//----------------------------------------------------------------------------
void verify_subsets(const smtk::mesh::CollectionPtr& c)
{
  std::vector< std::string > mesh_names = c->meshNames();

  smtk::mesh::MeshSet ms = c->meshes( mesh_names[0] );
  smtk::mesh::PointSet ps = ms.points();

  smtk::mesh::HandleRange range;

  std::set<smtk::mesh::Handle> set;
  std::vector<smtk::mesh::Handle> vec;

  for( smtk::mesh::HandleRange::pair_iterator iter = ps.range().begin();
       iter != ps.range().end(); ++iter )
     {
     range.insert( iter->first, iter->second - 1 );
     for ( smtk::mesh::Handle i=iter->first; i < iter->second; ++i)
       {
       set.insert(i);
       vec.push_back(i);
       }
     }

  smtk::mesh::PointSet ps2(c, range);
  smtk::mesh::PointSet ps3(c, set);
  smtk::mesh::PointSet ps4(c, vec);
  smtk::mesh::PointSet ps5(
    std::const_pointer_cast<const smtk::mesh::Collection>(c), range);

  test( ps  != ps2 );
  test( ps2 == ps3 );
  test( ps3 == ps4 );
  test( ps4 == ps5 );
}

//----------------------------------------------------------------------------
void verify_comparisons(const smtk::mesh::CollectionPtr& c)
{
  std::vector< std::string > mesh_names = c->meshNames();

  smtk::mesh::PointSet one = c->meshes( mesh_names[0] ).points();
  smtk::mesh::PointSet two = c->meshes( mesh_names[1] ).points();

  test(one == one);
  test( !(one != one) );
  test(two != one);
  test( !(two == one) );

  smtk::mesh::PointSet one_a(one);
  test(one_a == one);

  smtk::mesh::PointSet two_b = one_a;
  two_b = two; //test assignment operator
  test(two_b == two);

  test(one_a != two_b);
}

//----------------------------------------------------------------------------
void verify_contains(const smtk::mesh::CollectionPtr& c)
{
  //need to verify that point_set contains actually works
  //I think we should grab the point connectivity from a cell set and
  //verify that each point id in the point connectivity returns true

  smtk::mesh::CellSet hexs = c->cells( smtk::mesh::Hexahedron );
  smtk::mesh::PointSet hexPoints = hexs.points();
  smtk::mesh::PointConnectivity hexConn = hexs.pointConnectivity();

  smtk::mesh::CellType cellType;
  int size=0;
  const smtk::mesh::Handle* points;
  for(hexConn.initCellTraversal();
      hexConn.fetchNextCell(cellType, size, points);)
    {
    for(int i=0; i < size; ++i)
      {
      const bool contains = hexPoints.contains(points[i]);
      test( contains );
      }
    }
}

//----------------------------------------------------------------------------
void verify_find(const smtk::mesh::CollectionPtr& c)
{
  //need to verify that point_set contains actually works
  //I think we should grab the point connectivity from a cell set and
  //verify that each point id find return a value between 0 and (size()-1)
  smtk::mesh::CellSet hexs = c->cells( smtk::mesh::Hexahedron );
  smtk::mesh::PointSet hexPoints = hexs.points();
  smtk::mesh::PointConnectivity hexConn = hexs.pointConnectivity();

  smtk::mesh::CellType cellType;
  int size=0;
  const smtk::mesh::Handle* points;
  for(hexConn.initCellTraversal();
      hexConn.fetchNextCell(cellType, size, points);)
    {
    for(int i=0; i < size; ++i)
      {
      const std::size_t loc = hexPoints.find(points[i]);
      test( loc < hexPoints.size() );
      }
    }
}

//----------------------------------------------------------------------------
void verify_get(const smtk::mesh::CollectionPtr& c)
{
  //this is really hard to test currently as we only have bulk gets() on
  //points. So for now we are just going to make sure the function doesn't
  //crash
  smtk::mesh::PointSet all_points = c->points();
  const std::size_t numCoords = 3 * all_points.size();

  std::vector<double> coords(numCoords);
  all_points.get(&coords[0]);

  for(int i=0; i < smtk::mesh::CellType_MAX; ++i )
    {
    smtk::mesh::CellType cellType = static_cast<smtk::mesh::CellType>(i);
    smtk::mesh::CellSet cells = c->cells( cellType );

    test( cells.points().get(&coords[0]) != cells.is_empty() );
    }
}

//----------------------------------------------------------------------------
void verify_float_get(const smtk::mesh::CollectionPtr& c)
{

  //this is really hard to test currently as we only have bulk gets() on
  //points. So for now we are just going to make sure the function doesn't
  //crash
  smtk::mesh::PointSet all_points = c->points();
  const std::size_t numCoords = 3 * all_points.size();

  std::vector<float> coords(numCoords);
  all_points.get(&coords[0]);

  for(int i=0; i < smtk::mesh::CellType_MAX; ++i )
    {
    smtk::mesh::CellType cellType = static_cast<smtk::mesh::CellType>(i);
    smtk::mesh::CellSet cells = c->cells( cellType );

    test( cells.points().get(&coords[0]) != cells.is_empty() );
    }
}

//----------------------------------------------------------------------------
void verify_set(const smtk::mesh::CollectionPtr& c)
{

  //current plan, is to fetch all the points, mark all the Y values
  //as zero, and set that back onto the data, and than finally verify
  //those values by getting again
  smtk::mesh::PointSet all_points = c->points();
  const std::size_t numCoords = 3 * all_points.size();

  std::vector<double> coords(numCoords);
  all_points.get(&coords[0]); //now test the double* interface

  //flatten in the Y ( and store a copy of the original Y values, so we can roll back )
  std::vector<double> original_y(all_points.size());
  for(std::size_t i=0, yi = 0; i < coords.size(); i+=3, ++yi)
    {
    original_y[yi] = coords[i+1];
    coords[i+1] = 0.0;
    }
  all_points.set(coords); //push the new point coords

  all_points.get(coords); //now test the std::vector interface
  for(std::size_t i=0; i < coords.size(); i+=3)
    {
    test( (coords[i+1] == 0.0) );
    }

  //now reset the Y values to original values
  for(std::size_t i=0, yi = 0; i < coords.size(); i+=3, ++yi)
    {
    coords[i+1] = original_y[yi];
    }
  all_points.set(coords); //roll back to original points
}

//----------------------------------------------------------------------------
void verify_float_set(const smtk::mesh::CollectionPtr& c)
{

  //current plan, is to fetch all the points, mark all the Y values
  //as zero, and set that back onto the data, and than finally verify
  //those values by getting again
  smtk::mesh::PointSet all_points = c->points();
  const std::size_t numCoords = 3 * all_points.size();

  std::vector<float> coords(numCoords);
  all_points.get(&coords[0]); //now test the double* interface

  //flatten in the Y ( and store a copy of the original Y values, so we can roll back )
  std::vector<float> original_y(all_points.size());
  for(std::size_t i=0, yi = 0; i < coords.size(); i+=3, ++yi)
    {
    original_y[yi] = coords[i+1];
    coords[i+1] = 0.0f;
    }
  all_points.set(coords); //push the new point coords

  all_points.get(coords); //now test the std::vector interface
  for(std::size_t i=0; i < coords.size(); i+=3)
    {
    test( (coords[i+1] == 0.0f) );
    }

  //now reset the Y values to original values
  for(std::size_t i=0, yi = 0; i < coords.size(); i+=3, ++yi)
    {
    coords[i+1] = original_y[yi];
    }
  all_points.set(coords); //roll back to original points
}

//----------------------------------------------------------------------------
void verify_pointset_intersect(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::PointSet all_points = c->points();

  { //intersection of self should produce self
    smtk::mesh::PointSet result = smtk::mesh::set_intersect(all_points,all_points);
    test( result == all_points, "Intersection of self should produce self" );
  }

  { //intersection with nothing should produce nothing
    smtk::mesh::PointSet no_points = c->meshes( "bad name string" ).points();
    smtk::mesh::PointSet result = smtk::mesh::set_intersect(all_points,no_points);
    test( result == no_points, "Intersection with nothing should produce nothing" );
  }

  //find meshes that have volume elements
  smtk::mesh::PointSet volumePoints = c->meshes( smtk::mesh::Dims3 ).points();

  //verify that the size of the intersection + size of difference
  //equal size
  smtk::mesh::PointSet intersect_result =
                      smtk::mesh::set_intersect( all_points, volumePoints );

  smtk::mesh::PointSet difference_result =
                      smtk::mesh::set_difference( all_points, volumePoints );

  const std::size_t summed_size =
                      intersect_result.size() + difference_result.size();
  test( summed_size == all_points.size(),
      "Size of intersect + difference needs to be the same as total\
       number of unique items" );

}

//----------------------------------------------------------------------------
void verify_pointset_union(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::PointSet all_points = c->points();

  { //union with self produces self
    smtk::mesh::PointSet result = smtk::mesh::set_union(all_points,all_points);
    test( result == all_points, "Union of self should produce self" );
  }


  { //union with nothing should produce self
    smtk::mesh::PointSet no_points = c->meshes( "bad name string" ).points();
    smtk::mesh::PointSet result = smtk::mesh::set_union(all_points,no_points);
    test( result == all_points, "Union with nothing should produce self" );
  }

  //construct empty meshset(s)
  smtk::mesh::PointSet union_output = c->meshes( "bad name string" ).points();
  //verify that append and union produce the same result
  for(int i=0; i<4; ++i)
    {
    smtk::mesh::DimensionType d( static_cast<smtk::mesh::DimensionType>(i) );
    union_output = smtk::mesh::set_union(union_output, c->meshes(d).points() );
    }

  test( union_output == all_points, "Result of union should be the same as all_points");
}

//----------------------------------------------------------------------------
void verify_pointset_subtract(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::PointSet all_points = c->points();

  { //subtract of self should produce empty
    smtk::mesh::PointSet result = smtk::mesh::set_difference(all_points,all_points);
    test( result.size() == 0, "Subtraction of self should produce nothing" );
    test( result != all_points, "Subtraction of self should produce nothing" );
  }

  { //subtract with nothing should produce self
    smtk::mesh::PointSet no_points = c->meshes( "bad name string" ).points();
    smtk::mesh::PointSet result = smtk::mesh::set_difference(all_points,no_points);
    test( result == all_points, "Subtraction with nothing should produce self" );
  }

  { //subtract with something from nothing should produce nothing
    smtk::mesh::PointSet no_points = c->meshes( "bad name string" ).points();
    smtk::mesh::PointSet result = smtk::mesh::set_difference(no_points,all_points);
    test( result == no_points, "Subtraction of something from nothing should nothing" );
  }

  //find meshes that have volume elements
  smtk::mesh::PointSet volumePoints = c->meshes( smtk::mesh::Dims3 ).points();

  std::size_t size_difference = all_points.size() - volumePoints.size();
  smtk::mesh::PointSet non_dim_meshes = smtk::mesh::set_difference(all_points, volumePoints);
  test( non_dim_meshes.size() == size_difference,
        "subtract of two meshes produced wrong size" );
}

//----------------------------------------------------------------------------
class CountPoints : public smtk::mesh::PointForEach
{
  //keep a physical count of number of ponts so that we can verify we
  //don't iterate over a point more than once
  int numPointsIteratedOver;
public:
  //--------------------------------------------------------------------------
  CountPoints( smtk::mesh::CollectionPtr collection ):
    numPointsIteratedOver(0)
    {
    }

  //--------------------------------------------------------------------------
  void forPoints(const smtk::mesh::HandleRange& pointIds,
                 std::vector<double>& xyz,
                 bool& coordinatesModified)
  {
  //verify the coordinates and the number of points match
  test( (xyz.size() == (pointIds.size() * 3) ) );

  coordinatesModified = false; //we are not modifying the coords

  typedef smtk::mesh::HandleRange::const_iterator c_it;
  for(c_it i = pointIds.begin(); i != pointIds.end(); ++i)
    { //we could just increment by size of pointIds, but I want
      //to have an example of how to do iteration over the point ids
    this->numPointsIteratedOver++;
    }
  }

  int numberOfPointsVisited() const { return numPointsIteratedOver; }
};

//----------------------------------------------------------------------------
void verify_pointset_for_each_read(const smtk::mesh::CollectionPtr& c)
{
  CountPoints functor(c);
  smtk::mesh::MeshSet volMeshes = c->meshes( smtk::mesh::Dims3 );
  smtk::mesh::for_each( volMeshes.points(), functor );
  test( functor.numberOfPointsVisited() == volMeshes.points().size() );
}


//----------------------------------------------------------------------------
class FlattenZ : public smtk::mesh::PointForEach
{
public:
  //--------------------------------------------------------------------------
  void forPoints(const smtk::mesh::HandleRange& pointIds,
                 std::vector<double>& xyz,
                 bool& coordinatesModified)
  {
  //verify the coordinates and the number of points match
  test( (xyz.size() == (pointIds.size() * 3) ) );

  //we are modifying the coords, so signal that the modifications are saved
  coordinatesModified = true;

  for(std::size_t offset = 0; offset < xyz.size(); offset+=3)
    {
    //make all Z values 0.0
    xyz[offset+2] = 0.0;
    }
  }
};

//----------------------------------------------------------------------------
class VerifyZ : public smtk::mesh::PointForEach
{
public:
  //--------------------------------------------------------------------------
  void forPoints(const smtk::mesh::HandleRange& pointIds,
                 std::vector<double>& xyz,
                 bool& coordinatesModified)
  {
  //verify the coordinates and the number of points match
  test( (xyz.size() == (pointIds.size() * 3) ) );

  for(std::size_t offset = 0; offset < xyz.size(); offset+=3)
    {
    //make all Z values 0.0
    test( (xyz[offset+2] == 0.0) );
    }
  }
};

//----------------------------------------------------------------------------
void verify_pointset_for_each_modify(const smtk::mesh::CollectionPtr& c)
{
  //first modify all the points to have 0 for the z value
  smtk::mesh::MeshSet volMeshes = c->meshes( smtk::mesh::Dims3 );
  FlattenZ functorA;
  smtk::mesh::for_each( volMeshes.points(), functorA );

  //now verify that the points all have a z-value of 0
  VerifyZ functorB;
  smtk::mesh::for_each( volMeshes.points(), functorB );
}

}

//----------------------------------------------------------------------------
int UnitTestPointSet(int, char** const)
{
  smtk::mesh::ManagerPtr mngr = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = load_mesh(mngr);

  verify_constructors(c);
  verify_subsets(c);
  verify_comparisons(c);

  verify_contains(c);
  verify_find(c);

  verify_get(c);
  verify_float_get(c);

  verify_set(c);
  verify_float_set(c);

  verify_pointset_intersect(c);
  verify_pointset_union(c);
  verify_pointset_subtract(c);

  verify_pointset_for_each_read(c);
  verify_pointset_for_each_modify(c);

  return 0;
}
