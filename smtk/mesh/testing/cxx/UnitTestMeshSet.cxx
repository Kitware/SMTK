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
  smtk::common::UUID entity = smtk::io::ImportMesh::intoManager(file_path, mngr);
  test( !entity.isNull(), "uuid shouldn't be invalid");

  smtk::mesh::CollectionPtr c = mngr->collection(entity);
  test( c->isValid(), "collection should be valid");

  return c;
}

//----------------------------------------------------------------------------
void verify_num_meshes(const smtk::mesh::CollectionPtr& c)
{
  std::size_t numMeshes = c->numberOfMeshes();

  test( numMeshes!=0, "dataset once loaded should have more than zero meshes");
  test( numMeshes == 53, "dataset once loaded should have 53 meshes");

  smtk::mesh::MeshSet all_meshes = c->meshes();
  test( numMeshes == all_meshes.size());
  test( all_meshes.is_empty() == false );
}

//----------------------------------------------------------------------------
void verify_constructors(const smtk::mesh::CollectionPtr& c)
{
  std::vector< std::string > mesh_names = c->meshNames();

  smtk::mesh::MeshSet ms = c->meshes( mesh_names[0] );

  smtk::mesh::MeshSet ms2(ms);
  smtk::mesh::MeshSet ms3 = c->meshes( "bad_name" );
  test( ms3.is_empty() == true );
  test( ms3.size() == 0 );

  test( ms.size() == ms2.size());
  test( ms.size() != ms3.size());

  ms3 = ms; //test assignment operator
  test( ms.size() == ms3.size());

  test( ms.is_empty() == false );
  test( ms2.is_empty() == false );
  test( ms3.is_empty() == false );
}

//----------------------------------------------------------------------------
void verify_comparisons(const smtk::mesh::CollectionPtr& c)
{
  std::vector< std::string > mesh_names = c->meshNames();

  smtk::mesh::MeshSet one = c->meshes( mesh_names[0] );
  smtk::mesh::MeshSet two = c->meshes( mesh_names[1] );

  test(one == one);
  test( !(one != one) );
  test(two != one);
  test( !(two == one) );

  smtk::mesh::MeshSet one_a(one);
  test(one_a == one);

  smtk::mesh::MeshSet two_b = one_a;
  two_b = two; //test assignment operator
  test(two_b == two);

  test(one_a != two_b);

}
//----------------------------------------------------------------------------
void verify_mesh_by_name(const smtk::mesh::CollectionPtr& c)
{
  std::vector< std::string > mesh_names = c->meshNames();
  std::size_t collec_numMeshes = c->numberOfMeshes();

  //while we can't state that every mesh will have a name, we do know
  //that at least 1 will have a name
  test( collec_numMeshes >= mesh_names.size() );
  test( mesh_names.size() > 0 );

  //now iterate over all the mesh_name and verify we can
  //get a valid meshset out of it
  typedef std::vector< std::string >::const_iterator it;
  std::size_t numMeshesWithNames = 0;
  for(it i = mesh_names.begin(); i != mesh_names.end(); ++i)
    {
    std::cout << "Looking for mesh: " << *i << std::endl;
    smtk::mesh::MeshSet ms = c->meshes( *i );
    test(ms.size() != 0);
    numMeshesWithNames += ms.size();
    }

  test(collec_numMeshes >= numMeshesWithNames,
     "Number of meshes with names should be less than total number of meshes");
}

//----------------------------------------------------------------------------
void verify_meshset_by_dim(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::MeshSet all_meshes = c->meshes();

  //verify that the meshset given back from the collection is the same size
  //as the number of meshes in the entire dataset
  std::size_t collec_numMeshes = c->numberOfMeshes();
  std::size_t size = all_meshes.size();
  test( size == collec_numMeshes );

  //query for all meshsets of dimension 0, 1, 2, and 3
  std::size_t numMeshesFoundByDimCalls = 0;
  for(int i=0; i<4; ++i)
    {

    smtk::mesh::DimensionType d( static_cast<smtk::mesh::DimensionType>(i) );
    smtk::mesh::MeshSet meshesWithDim = c->meshes( d );
    numMeshesFoundByDimCalls += meshesWithDim.size();
    }

  //generally we will have material and boundary condition meshes which
  //won't be labeled with a dimension, so we expect numMeshesFoundByDimCalls
  //to be less than the total number of meshes
  test( all_meshes.size() >= numMeshesFoundByDimCalls );
}

//----------------------------------------------------------------------------
void verify_meshset_intersect(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::MeshSet all_meshes = c->meshes();

  { //intersection of self should produce self
    smtk::mesh::MeshSet result = smtk::mesh::set_intersect(all_meshes,all_meshes);
    test( result == all_meshes, "Intersection of self should produce self" );
  }

  { //intersection with nothing should produce nothing
    smtk::mesh::MeshSet no_meshes = c->meshes( "bad name string" );
    smtk::mesh::MeshSet result = smtk::mesh::set_intersect(all_meshes,no_meshes);
    test( result == no_meshes, "Intersection with nothing should produce nothing" );
  }

  //construct empty meshset
  smtk::mesh::MeshSet all_dims = c->meshes( "bad name string" );
  for(int i=0; i<4; ++i)
    {
    smtk::mesh::DimensionType d( static_cast<smtk::mesh::DimensionType>(i) );
    smtk::mesh::MeshSet meshesWithDim = c->meshes(d);

    //all_dims shouldn't already hold anything from meshesWithDim
    smtk::mesh::MeshSet intersect_result =
                      smtk::mesh::set_intersect(all_dims,meshesWithDim);
    test( intersect_result.size() == 0 );

    all_dims.append( meshesWithDim );
    }

  //verify that the size of the intersection + size of difference
  //equal size
  smtk::mesh::MeshSet intersect_result =
                      smtk::mesh::set_intersect( all_meshes, all_dims );

  smtk::mesh::MeshSet difference_result =
                      smtk::mesh::set_difference( all_meshes, all_dims );

  const std::size_t summed_size =
                      intersect_result.size() + difference_result.size();
  test( summed_size == all_meshes.size(),
      "Size of intersect + difference needs to be the same as total\
       number of unique items" );

}

//----------------------------------------------------------------------------
void verify_meshset_union(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::MeshSet all_meshes = c->meshes();

  { //union with self produces self
    smtk::mesh::MeshSet result = smtk::mesh::set_union(all_meshes,all_meshes);
    test( result == all_meshes, "Union of self should produce self" );
  }


  { //union with nothing should produce self
    smtk::mesh::MeshSet no_meshes = c->meshes( "bad name string" );
    smtk::mesh::MeshSet result = smtk::mesh::set_union(all_meshes,no_meshes);
    test( result == all_meshes, "Union with nothing should produce self" );
  }

  //construct empty meshset(s)
  smtk::mesh::MeshSet all_dims = c->meshes( "bad name string" );
  smtk::mesh::MeshSet append_output = all_dims;
  //verify that append and union produce the same result
  for(int i=0; i<4; ++i)
    {
    smtk::mesh::DimensionType d( static_cast<smtk::mesh::DimensionType>(i) );
    all_dims = smtk::mesh::set_union(all_dims, c->meshes(d) );
    append_output.append( c->meshes(d) );
    }

  test( all_dims == append_output, "Result of union should be the same as append");

  {
    smtk::mesh::MeshSet result = smtk::mesh::set_union(all_meshes,all_dims);
    smtk::mesh::MeshSet result2 = all_meshes;
    result2.append(all_dims);
    test( result == result2);
  }

}

//----------------------------------------------------------------------------
void verify_meshset_subtract(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::MeshSet all_meshes = c->meshes();

  { //subtract of self should produce empty
    smtk::mesh::MeshSet result = smtk::mesh::set_difference(all_meshes,all_meshes);
    test( result.size() == 0, "Subtraction of self should produce nothing" );
    test( result != all_meshes, "Subtraction of self should produce nothing" );
  }

  { //subtract with nothing should produce self
    smtk::mesh::MeshSet no_meshes = c->meshes( "bad name string" );
    smtk::mesh::MeshSet result = smtk::mesh::set_difference(all_meshes,no_meshes);
    test( result == all_meshes, "Subtraction with nothing should produce self" );
  }

  { //subtract with something from nothing should produce nothing
    smtk::mesh::MeshSet no_meshes = c->meshes( "bad name string" );
    smtk::mesh::MeshSet result = smtk::mesh::set_difference(no_meshes,all_meshes);
    test( result == no_meshes, "Subtraction of something from nothing should nothing" );
  }

  //construct empty meshset
  smtk::mesh::MeshSet all_dims = c->meshes( "bad name string" );
  for(int i=0; i<4; ++i)
    {
    smtk::mesh::DimensionType d( static_cast<smtk::mesh::DimensionType>(i) );
    all_dims.append( c->meshes(d) );
    }

  std::size_t size_difference = all_meshes.size() - all_dims.size();
  smtk::mesh::MeshSet non_dim_meshes = smtk::mesh::set_difference(all_meshes, all_dims);
  test( non_dim_meshes.size() == size_difference,
        "subtract of two meshes produced wrong size" );
}

}

//----------------------------------------------------------------------------
int UnitTestMeshSet(int argc, char** argv)
{
  smtk::mesh::ManagerPtr mngr = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = load_mesh(mngr);

  verify_num_meshes(c);
  verify_constructors(c);
  verify_comparisons(c);
  verify_mesh_by_name(c);
  verify_meshset_by_dim(c);
  verify_meshset_intersect(c);
  verify_meshset_union(c);
  verify_meshset_subtract(c);

  return 0;
}