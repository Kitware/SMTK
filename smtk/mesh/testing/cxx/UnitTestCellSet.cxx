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
  file_path += "/mesh/3d/twoassm_out.h5m";

  smtk::mesh::CollectionPtr c  = smtk::io::importMesh(file_path, mngr);
  test( c->isValid(), "collection should be valid");
  return c;
}

//----------------------------------------------------------------------------
void verify_constructors(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::MeshSet all_meshes = c->meshes();

  smtk::mesh::CellSet all_cells_from_ms = all_meshes.cells();
  smtk::mesh::CellSet copy_of_all_cells(all_cells_from_ms);

  test( all_cells_from_ms.is_empty() == false );
  test( all_cells_from_ms.size() != 0 );
  test( all_cells_from_ms.size() == copy_of_all_cells.size());

  smtk::mesh::CellSet zeroDim =  c->cells( smtk::mesh::Dims0 );
  smtk::mesh::CellSet equalToZeroDim = copy_of_all_cells;
  equalToZeroDim = zeroDim; //test assignment operator
  test( equalToZeroDim.size() == zeroDim.size());
  test( equalToZeroDim.is_empty() == false );
}

//----------------------------------------------------------------------------
void verify_subsets(const smtk::mesh::CollectionPtr& c)
{
  std::vector< std::string > mesh_names = c->meshNames();

  test( mesh_names.empty() == false, "There are no meshes in the collection.");

  smtk::mesh::MeshSet ms = c->meshes( mesh_names[0] );
  smtk::mesh::CellSet ps = ms.cells();

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

  smtk::mesh::CellSet ps2(c, range);
  smtk::mesh::CellSet ps3(c, set);
  smtk::mesh::CellSet ps4(c, vec);
  smtk::mesh::CellSet ps5(
    std::const_pointer_cast<const smtk::mesh::Collection>(c), range);

  test( ps  != ps2 );
  test( ps2 == ps3 );
  test( ps3 == ps4 );
  test( ps4 == ps5 );
}

//----------------------------------------------------------------------------
void verify_empty(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::MeshSet no_mesh = c->meshes( "bad name string" );

  smtk::mesh::CellSet no_cells_a = no_mesh.cells();
  smtk::mesh::CellSet no_cells_b = no_mesh.cells( smtk::mesh::Hexahedron );
  smtk::mesh::CellSet no_cells_c = no_mesh.cells( smtk::mesh::Dims2 );
  smtk::mesh::CellSet no_cells_d = no_mesh.cells( smtk::mesh::CellTypes() );

  test( no_cells_a.is_empty() == true );
  test( no_cells_b.is_empty() == true );
  test( no_cells_c.is_empty() == true );
  test( no_cells_d.is_empty() == true );

  test( no_cells_a.size() == 0 );
  test( no_cells_b.size() == 0 );
  test( no_cells_c.size() == 0 );
  test( no_cells_d.size() == 0 );

}

//----------------------------------------------------------------------------
void verify_comparisons(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::CellSet zeroDim =  c->cells( smtk::mesh::Dims0 );
  smtk::mesh::CellSet oneDim =  c->cells( smtk::mesh::Dims1 );


  test(zeroDim == zeroDim);
  test( !(zeroDim != zeroDim) );
  test(oneDim != zeroDim);
  test( !(oneDim == zeroDim) );

  smtk::mesh::CellSet zeroDim_a(zeroDim);
  test(zeroDim_a == zeroDim);

  smtk::mesh::CellSet oneDim_b = zeroDim_a;
  oneDim_b = oneDim; //test assignment operator
  test(oneDim_b == oneDim);

  test(zeroDim_a != oneDim_b);

}

//----------------------------------------------------------------------------
void verify_typeset(const smtk::mesh::CollectionPtr& c)
{
  //verify that empty cell set has empty type set
  {
  smtk::mesh::CellTypes no_cell_types;
  smtk::mesh::CellSet emptyCellSet = c->cells( no_cell_types );
  smtk::mesh::TypeSet noTypes = emptyCellSet.types();

  test( noTypes.cellTypes() == no_cell_types);
  test( noTypes.hasMeshes() == false);
  test( noTypes.hasCells() == false);
  }


  //verify that if we get all cells from the collection the type set is correct
  {
  smtk::mesh::TypeSet all_types = c->types();
  smtk::mesh::CellSet allCells = c->cells( all_types.cellTypes() );
  smtk::mesh::TypeSet allCellsTypes = allCells.types();

  test( allCellsTypes.cellTypes() == all_types.cellTypes());
  test( allCellsTypes.hasMeshes() == false);
  test( allCellsTypes.hasCells() == true);
  }

  //verify typeset work on dimension cell queries and cell type queries
  //is handled by the methods verify_cell_count_by_dim and
  //verify_cell_count_by_type
}

//----------------------------------------------------------------------------
void verify_all_cells(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::MeshSet all_meshes = c->meshes();
  smtk::mesh::CellSet all_cells_from_collec = c->cells();
  smtk::mesh::CellSet all_cells_from_ms = all_meshes.cells();

  test( all_cells_from_collec.is_empty() == false);
  test( all_cells_from_ms.is_empty() == false);

  test( all_cells_from_collec.size() != 0);
  test( all_cells_from_ms.size() != 0);

  test( all_cells_from_collec.size() == all_cells_from_ms.size());
  test( all_cells_from_collec == all_cells_from_ms);
}

//----------------------------------------------------------------------------
void verify_cell_count_by_type(smtk::mesh::MeshSet ms)
{
  //verify that all cells size is equal to the vector of cellsets where
  //each element is a cell type.
  std::vector< smtk::mesh::CellSet > all_cell_types;
  smtk::mesh::TypeSet all_typeset;

  for(int i=0; i < smtk::mesh::CellType_MAX; ++i )
    {
    smtk::mesh::CellType cellType = static_cast<smtk::mesh::CellType>(i);
    smtk::mesh::CellSet cells = ms.cells( cellType );
    all_cell_types.push_back( cells );

    //verify that the cells typeset is correct
    smtk::mesh::TypeSet types = cells.types();
    test( types.hasMeshes() == false);
    test( types.hasCells() == (!cells.is_empty()) );
    if(!cells.is_empty())
      { //only do these tests if we have anything in the cellset
      test( types.hasCell( cellType ) == true );
      }
    all_typeset += types;
    }

  std::size_t sum = 0;
  for(std::size_t i=0; i < all_cell_types.size(); ++i )
    {
    sum += all_cell_types[i].size();
    }
  test( ms.cells().size() == sum);
  test( ms.types().cellTypes() == all_typeset.cellTypes());
}

//----------------------------------------------------------------------------
void verify_cell_count_by_dim(smtk::mesh::MeshSet ms)
{
  //verify that all cells size is equal to the vector of cellsets where
  //each element is a cell type.
  std::vector< smtk::mesh::CellSet > all_cell_types;
  smtk::mesh::TypeSet all_typeset;

  for(int i=0; i < smtk::mesh::DimensionType_MAX; ++i )
    {
    smtk::mesh::DimensionType dimType = static_cast<smtk::mesh::DimensionType>(i);
    smtk::mesh::CellSet cells = ms.cells( dimType );
    all_cell_types.push_back( cells );

    //verify that the cells typeset is correct
    smtk::mesh::TypeSet types = cells.types();
    test( types.hasMeshes() == false);
    test( types.hasCells() == (!cells.is_empty()) );
    if(!cells.is_empty())
      { //only do these tests if we have anything in the cellset
      test( types.hasDimension( dimType ) == true );
      }
    all_typeset += types;
    }

  std::size_t sum = 0;
  for(std::size_t i=0; i < all_cell_types.size(); ++i )
    {
    sum += all_cell_types[i].size();
    }
  test( ms.cells().size() == sum);
  test( ms.types().cellTypes() == all_typeset.cellTypes());
}


//----------------------------------------------------------------------------
void verify_cells_by_type(const smtk::mesh::CollectionPtr& c)
{
  //1. verify that all cells size is equal to the vector of cellsets where
  //each element is a cell type.
  smtk::mesh::CellSet all_cells = c->cells();
  smtk::mesh::MeshSet all_meshes = c->meshes();

  std::vector< smtk::mesh::CellSet > all_cell_types;
  smtk::mesh::CellSet all_cells_appended = all_meshes.cells( smtk::mesh::Vertex );
  for(int i=0; i < smtk::mesh::CellType_MAX; ++i )
    {
    smtk::mesh::CellType cellType = static_cast<smtk::mesh::CellType>(i);
    all_cell_types.push_back(  c->cells( cellType ) );
    all_cells_appended.append( all_meshes.cells( cellType ) );
    }
  test( all_cells_appended == all_cells );

  std::size_t sum = 0;
  for(std::size_t i=0; i < all_cell_types.size(); ++i )
    {
    sum += all_cell_types[i].size();

    //should be a no-op verifies append doesn't double add
    all_cells_appended.append( all_cell_types[i] );
    }

  test( all_cells.size() == sum);
  test( all_cells_appended == all_cells );

  //2. Verify that for each cell type in associated types the related index
  //in the array has cells

  smtk::mesh::TypeSet types = all_cells.types();
  for(int i=0; i < smtk::mesh::CellType_MAX; ++i )
    {
    //verify that if the collection has a cell, we also return
    smtk::mesh::CellType ct = static_cast<smtk::mesh::CellType>(i);
    test( types.hasCell( ct ) != all_cell_types[i].is_empty() );
    }

  //now verify certain mesh subset
  verify_cell_count_by_type( c->meshes() );
  verify_cell_count_by_dim( c->meshes() );

  verify_cell_count_by_type( c->meshes( smtk::mesh::Dims1 ) );
  verify_cell_count_by_type( c->meshes( smtk::mesh::Dims2 ) );
  verify_cell_count_by_type( c->meshes( smtk::mesh::Dims3 ) );

  verify_cell_count_by_dim( c->meshes( smtk::mesh::Dims1 ) );
  verify_cell_count_by_dim( c->meshes( smtk::mesh::Dims2 ) );
  verify_cell_count_by_dim( c->meshes( smtk::mesh::Dims3 ) );
}


//----------------------------------------------------------------------------
void verify_cells_by_types(const smtk::mesh::CollectionPtr& c)
{

  //verify that empty typeset returns nothing
  smtk::mesh::CellTypes no_cell_types;
  smtk::mesh::CellSet no_associatedCells = c->cells( no_cell_types );
  test( no_associatedCells.is_empty() == true ); //should be empty

  //1. verify that when we query based on types everything works properly
  //when from a Collection
  smtk::mesh::TypeSet types = c->types();
  smtk::mesh::CellSet associatedCells = c->cells( types.cellTypes() );
  test( associatedCells.is_empty() == false ); //can't be false

  //verify cellTypes returns the same number of cells as asking for all cells
  test( associatedCells.size() == c->cells().size() );
  test( associatedCells == c->cells() );
}

//----------------------------------------------------------------------------
void verify_cells_by_dim(const smtk::mesh::CollectionPtr& c)
{

  //simple verification that the types are the same
  smtk::mesh::MeshSet all_meshes = c->meshes();

  smtk::mesh::CellSet zeroDim  =   c->cells( smtk::mesh::Dims0 );
  smtk::mesh::CellSet oneDim   =   c->cells( smtk::mesh::Dims1 );
  smtk::mesh::CellSet twoDim   =   c->cells( smtk::mesh::Dims2 );
  smtk::mesh::CellSet threeDim =   c->cells( smtk::mesh::Dims3 );

  //fm = from meshset
  smtk::mesh::CellSet fm_zeroDim  =   all_meshes.cells( smtk::mesh::Dims0 );
  smtk::mesh::CellSet fm_oneDim   =   all_meshes.cells( smtk::mesh::Dims1 );
  smtk::mesh::CellSet fm_twoDim   =   all_meshes.cells( smtk::mesh::Dims2 );
  smtk::mesh::CellSet fm_threeDim =   all_meshes.cells( smtk::mesh::Dims3 );

  test( zeroDim  == fm_zeroDim );
  test( oneDim   == fm_oneDim );
  test( twoDim   == fm_twoDim );
  test( threeDim == fm_threeDim );

}

//----------------------------------------------------------------------------
void verify_cellset_intersect(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::MeshSet no_meshes = c->meshes( "bad name string" );
  smtk::mesh::MeshSet all_meshes = c->meshes();
  smtk::mesh::CellSet all_cells = c->cells();

  { //intersection of self should produce self
    smtk::mesh::CellSet result = smtk::mesh::set_intersect(all_cells,all_cells);
    test( result == all_cells, "Intersection of self should produce self" );
  }

  { //intersection with nothing should produce nothing
    smtk::mesh::CellSet no_cells = no_meshes.cells();
    smtk::mesh::CellSet result = smtk::mesh::set_intersect(all_cells,no_cells);
    test( result == no_cells, "Intersection with nothing should produce nothing" );
  }

  //construct empty meshset
  smtk::mesh::CellSet all_dims = no_meshes.cells();
  for(int i=0; i< smtk::mesh::DimensionType_MAX; ++i)
    {
    smtk::mesh::DimensionType d( static_cast<smtk::mesh::DimensionType>(i) );
    smtk::mesh::CellSet cellsWithDim = all_meshes.cells(d);

    //all_dims shouldn't already hold anything from cellsWithDim
    smtk::mesh::CellSet intersect_result =
                      smtk::mesh::set_intersect(all_dims,cellsWithDim);
    test( intersect_result.size() == 0 );

    all_dims.append( cellsWithDim );
    }

  //verify that the size of the intersection + size of difference
  //equal size
  smtk::mesh::CellSet intersect_result =
                      smtk::mesh::set_intersect( all_cells, all_dims );

  smtk::mesh::CellSet difference_result =
                      smtk::mesh::set_difference( all_cells, all_dims );

  const std::size_t summed_size =
                      intersect_result.size() + difference_result.size();
  test( summed_size == all_cells.size(),
      "Size of intersect + difference needs to be the same as total\
       number of unique items" );

}

//----------------------------------------------------------------------------
void verify_cellset_union(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::MeshSet all_meshes = c->meshes();
  smtk::mesh::CellSet all_cells = c->cells();

  { //union with self produces self
    smtk::mesh::CellSet result = smtk::mesh::set_union(all_cells,all_cells);
    test( result == all_cells, "Union of self should produce self" );
  }


  { //union with nothing should produce self
    smtk::mesh::MeshSet no_meshes = c->meshes( "bad name string" );
    smtk::mesh::CellSet no_cells = no_meshes.cells();
    smtk::mesh::CellSet result = smtk::mesh::set_union(all_cells,no_cells);
    test( result == all_cells, "Union with nothing should produce self" );
  }

  //construct empty meshset(s)
  smtk::mesh::CellSet all_dims = c->meshes( "bad name string" ).cells();
  smtk::mesh::CellSet append_output = all_dims;
  //verify that append and union produce the same result
  for(int i=0; i< smtk::mesh::DimensionType_MAX; ++i)
    {
    smtk::mesh::DimensionType d( static_cast<smtk::mesh::DimensionType>(i) );
    all_dims = smtk::mesh::set_union(all_dims, c->cells(d) );
    append_output.append( all_meshes.cells(d) );
    }

  test( all_dims == append_output, "Result of union should be the same as append");

  {
    smtk::mesh::CellSet result = smtk::mesh::set_union(all_cells,all_dims);
    smtk::mesh::CellSet result2 = all_cells;
    result2.append(all_dims);
    test( result == result2);
  }

}

//----------------------------------------------------------------------------
void verify_cellset_subtract(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::CellSet all_cells = c->cells();

  { //subtract of self should produce empty
    smtk::mesh::CellSet result = smtk::mesh::set_difference(all_cells,all_cells);
    test( result.size() == 0, "Subtraction of self should produce nothing" );
    test( result != all_cells, "Subtraction of self should produce nothing" );
  }

  { //subtract with nothing should produce self
    smtk::mesh::CellSet no_cells = c->meshes( "bad name string" ).cells();
    smtk::mesh::CellSet result = smtk::mesh::set_difference(all_cells,no_cells);
    test( result == all_cells, "Subtraction with nothing should produce self" );
  }

  { //subtract with something from nothing should produce nothing
    smtk::mesh::CellSet no_cells = c->meshes( "bad name string" ).cells();
    smtk::mesh::CellSet result = smtk::mesh::set_difference(no_cells,all_cells);
    test( result == no_cells, "Subtraction of something from nothing should nothing" );
  }

  //construct empty meshset
  smtk::mesh::CellSet all_dims = c->meshes( "bad name string" ).cells();
  for(int i=0; i< smtk::mesh::DimensionType_MAX; ++i)
    {
    smtk::mesh::DimensionType d( static_cast<smtk::mesh::DimensionType>(i) );
    all_dims.append( c->cells(d) );
    }

  std::size_t size_difference = all_cells.size() - all_dims.size();
  smtk::mesh::CellSet non_dim_meshes = smtk::mesh::set_difference(all_cells, all_dims);
  test( non_dim_meshes.size() == size_difference,
        "subtract of two meshes produced wrong size" );
}

//----------------------------------------------------------------------------
void verify_cellset_point_intersect(const smtk::mesh::CollectionPtr& c)
{
  using namespace smtk::mesh;

  smtk::mesh::CellSet twoDim   =   c->cells( smtk::mesh::Dims2 );
  smtk::mesh::CellSet threeDim =   c->cells( smtk::mesh::Dims3 );

  //First test partial containment
  {
    smtk::mesh::CellSet result =
      smtk::mesh::point_intersect(threeDim, threeDim, PartiallyContained );
    test( result == threeDim, "intersection of self should produce self " );
  }

  {
    smtk::mesh::CellSet result =
      smtk::mesh::point_intersect(threeDim, twoDim, PartiallyContained );
    test( result.size() != 0, "we should have partial containment between 2d and 3d" );
  }

  {
    smtk::mesh::CellSet result =
      smtk::mesh::point_intersect(twoDim, threeDim, PartiallyContained );
    test( result.size() != 0, "we should have partial containment between 3d and 2d" );
  }

  //Next test full containment
  {
    smtk::mesh::CellSet result =
      smtk::mesh::point_intersect(threeDim, threeDim, FullyContained );
    test( result == threeDim, "intersection of self should produce self " );
  }

  {
    smtk::mesh::CellSet result =
      smtk::mesh::point_intersect(threeDim, twoDim, FullyContained );
    test( result.size() != 0, "we should have partial containment between 2d and 3d" );
  }

  {
    smtk::mesh::CellSet result =
      smtk::mesh::point_intersect(twoDim, threeDim, FullyContained );
    test( result.size() == 0, "we should have no threeDim elements fully covered by 2d" );
  }

}

//----------------------------------------------------------------------------
void verify_cellset_point_difference(const smtk::mesh::CollectionPtr& c)
{
  using namespace smtk::mesh;

  smtk::mesh::CellSet twoDim   =   c->cells( smtk::mesh::Dims2 );
  smtk::mesh::CellSet threeDim =   c->cells( smtk::mesh::Dims3 );

  //First test partial containment
  {
    smtk::mesh::CellSet result =
      smtk::mesh::point_difference(threeDim, threeDim, PartiallyContained );
    test( result.size() == 0, "difference of self should produce nothing " );
  }

  {
    smtk::mesh::CellSet result =
      smtk::mesh::point_difference(threeDim, twoDim, PartiallyContained );
    test( result.size() == 0, "all points in the 2d cells should be used by a 3d cell" );
  }

  {
    smtk::mesh::CellSet result =
      smtk::mesh::point_difference(twoDim, threeDim, PartiallyContained );
    test( result.size() != 0, "every 3d cell has a point used by a 2d cell?" );
  }

  //Next test full containment
  {
    smtk::mesh::CellSet result =
      smtk::mesh::point_difference(threeDim, threeDim, FullyContained );
    test( result.size() == 0, "difference of self should produce nothing " );
  }

  {
    smtk::mesh::CellSet result =
      smtk::mesh::point_difference(threeDim, twoDim, FullyContained );
    test( result.size() == 0, "all points in the 2d cells should be used by a 3d cell" );
  }

  {
    smtk::mesh::CellSet result =
      smtk::mesh::point_difference(twoDim, threeDim, FullyContained );
    test( result.size() != 0, "not all points in the 3d cells should be used by 2d cell's" );
  }
}


//----------------------------------------------------------------------------
class CountCells : public smtk::mesh::CellForEach
{
  //keep the range of points we have seen so we can verify that we
  //seen all the cells that we expect to be given
  smtk::mesh::HandleRange pointsSeen;

  //keep the range of cells we have seen so we can verify that we
  //seen all the cells that we expect to be given
  smtk::mesh::HandleRange cellsSeen;

  //keep track of all the cell types we have been passed
  smtk::mesh::CellTypes cellTypesSeen;

  //keep a physical count of number of cells so that we can verify we
  //don't iterate over a cell more than once
  int numCellsVisited;

  //total number of points seen, relates to the total size of the connectivity
  //array for this cellset
  int numPointsSeen;
public:
  //--------------------------------------------------------------------------
  CountCells( ):
    smtk::mesh::CellForEach(),
    pointsSeen( ),
    cellsSeen(),
    numCellsVisited(0),
    numPointsSeen(0)
    {
    }

  //--------------------------------------------------------------------------
  void forCell(const smtk::mesh::Handle& cellId,
               smtk::mesh::CellType cellType, int numPts)
  {
  this->cellsSeen.insert(cellId);
  this->numCellsVisited++;
  this->numPointsSeen += numPts;
  this->pointsSeen.insert( this->pointIds(),
                           this->pointIds()+numPts);
  this->cellTypesSeen[static_cast<int>(cellType)] = true;
  }

  int numberOCellsVisited() const { return numCellsVisited; }
  int numberOPointsSeen() const { return numPointsSeen; }

  smtk::mesh::HandleRange points() const { return pointsSeen; }
  smtk::mesh::HandleRange cells() const { return cellsSeen; }

  smtk::mesh::CellTypes cellTypes() const { return cellTypesSeen; }
};

//----------------------------------------------------------------------------
void verify_cellset_for_each(const smtk::mesh::CollectionPtr& c)
{
  CountCells functor;
  smtk::mesh::MeshSet volMeshes = c->meshes( smtk::mesh::Dims3 );
  smtk::mesh::for_each( volMeshes.cells(), functor );

  test( static_cast<std::size_t>(functor.numberOCellsVisited()) ==
        volMeshes.cells().size() );
  test( static_cast<std::size_t>(functor.numberOPointsSeen()) ==
        volMeshes.pointConnectivity().size() );

  smtk::mesh::PointConnectivity pc = volMeshes.pointConnectivity();
  pc.initCellTraversal();

  int numPts;
  int numPointsSeen = 0;
  smtk::mesh::HandleRange pointsFromConnectivity;
  const smtk::mesh::Handle* points;
  while( pc.fetchNextCell(numPts, points) )
    {
    numPointsSeen += numPts;
    pointsFromConnectivity.insert(points, points+numPts);
    }

  //verify that point connectivity iteration and cell for_each visit
  //all the same cells, and we also see all the points
  test( volMeshes.cells() == smtk::mesh::CellSet(c,functor.cells()) );
  test( pointsFromConnectivity == functor.points() );
  test( numPointsSeen == functor.numberOPointsSeen() );


  //verify that the cell types that are reported are only 3D cells.
  smtk::mesh::TypeSet typeSet( functor.cellTypes(), false, true );
  test( typeSet.hasDimension( smtk::mesh::Dims1 ) == false );
  test( typeSet.hasDimension( smtk::mesh::Dims2 ) == false );
  test( typeSet.hasDimension( smtk::mesh::Dims3 ) == true );
}


}

//----------------------------------------------------------------------------
int UnitTestCellSet(int, char** const)
{
  smtk::mesh::ManagerPtr mngr = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = load_mesh(mngr);

  verify_constructors(c);
  verify_subsets(c);
  verify_empty(c);
  verify_comparisons(c);
  verify_typeset(c);
  verify_all_cells(c);
  verify_cells_by_type(c);
  verify_cells_by_types(c);
  verify_cells_by_dim(c);
  verify_cellset_intersect(c);
  verify_cellset_union(c);
  verify_cellset_subtract(c);

  verify_cellset_point_intersect(c);
  verify_cellset_point_difference(c);

  verify_cellset_for_each(c);

  return 0;
}
