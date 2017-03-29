//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ImportMesh.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/TypeSet.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

//----------------------------------------------------------------------------
void verify_basic_typeset()
{
  smtk::mesh::TypeSet t_set;
  test( t_set.hasMeshes() == false );
  test( t_set.hasCells() == false );

  //verify all cell types are zero
  for(int i=0; i < smtk::mesh::CellType_MAX; ++i)
    {
    test( t_set.hasCell( static_cast<smtk::mesh::CellType>(i) ) == false );
    }

  //verify all dim types are zero
  for(int i=0; i < 3; ++i)
    {
    test( t_set.hasDimension( static_cast<smtk::mesh::DimensionType>(i) ) == false );
    }
}

//----------------------------------------------------------------------------
void verify_typeset_constructors()
{
  //just verify that all the copy constructors work
  smtk::mesh::TypeSet t_set(smtk::mesh::CellTypes(),
                           false, true ); //has cells only
  smtk::mesh::TypeSet t_set2(t_set);
  smtk::mesh::TypeSet t_set3 = t_set2;

  test( t_set3.hasMeshes() == false );
  test( t_set3.hasCells() == true );

  //verify all cell types are zero
  for(int i=0; i < smtk::mesh::CellType_MAX; ++i)
    {
    test( t_set.hasCell( static_cast<smtk::mesh::CellType>(i) ) == false );
    }

  //verify all dim types are zero
  for(int i=0; i < 3; ++i)
    {
    test( t_set.hasDimension( static_cast<smtk::mesh::DimensionType>(i) ) == false );
    }

}

//----------------------------------------------------------------------------
void verify_typeset_cellTypes()
{
  //now verify a typeset with a couple things set
  smtk::mesh::TypeSet t_set(smtk::mesh::CellTypes( std::string("11100") ), //triangle,quad,polygon
                            false, true ); //has cells only

  const smtk::mesh::CellTypes correct_types( std::string("11100") );

  test( correct_types == t_set.cellTypes() );
}

//----------------------------------------------------------------------------
void verify_typeset_with_cells_and_dims()
{
  //now verify a typeset with a couple things set
  smtk::mesh::TypeSet t_set(smtk::mesh::CellTypes( std::string("11100") ), //triangle,quad,polygon
                            false, true ); //has cells only

  test( t_set.hasMeshes() == false );
  test( t_set.hasCells() == true );

  //verify all only 2d cells are valid
  for(int i=0; i < smtk::mesh::CellType_MAX; ++i)
    {
    bool expected_result = false;
    smtk::mesh::CellType currentCellType = static_cast<smtk::mesh::CellType>(i);
    if( currentCellType >= smtk::mesh::Triangle &&
        currentCellType <= smtk::mesh::Polygon )
      {
      expected_result = true;
      }

    const bool result = t_set.hasCell( currentCellType );
    test( result == expected_result );
    }

  //verify only 2d only is true
  test( t_set.hasDimension( smtk::mesh::Dims0 ) == false);
  test( t_set.hasDimension( smtk::mesh::Dims1 ) == false);
  test( t_set.hasDimension( smtk::mesh::Dims2 ) == true);
  test( t_set.hasDimension( smtk::mesh::Dims3 ) == false);

}

}

//----------------------------------------------------------------------------
int UnitTestTypeSet(int, char** const)
{
  //first lets test the very basic interactions.
  verify_basic_typeset();

  verify_typeset_constructors();
  verify_typeset_cellTypes();
  verify_typeset_with_cells_and_dims();

  return 0;
}
