//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/QueryTypes.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

struct verify_all_off_cell_types
{
  smtk::mesh::CellTypes ctypes;
  verify_all_off_cell_types()
    : ctypes()
  { //initialize the cellTypes for this struct to be all off
  }

  void operator()(smtk::mesh::CellType cellEnum)
  { //this operator will be called for each CellType, so verify that
    //the struct 's ctypes bit is turned off for the given type
    test(ctypes[cellEnum] == false, "This cell type should be off");
  }
};

struct verify_all_on_cell_types
{
  smtk::mesh::CellTypes ctypes;
  verify_all_on_cell_types()
    : ctypes(1023) //(2**10) -1
  {                //initialize the cellTypes for this struct to be all off
  }

  void operator()(smtk::mesh::CellType cellEnum)
  { //this operator will be called for each CellType, so verify that
    //the struct 's ctypes bit is turned off for the given type
    test(ctypes[cellEnum] == true, "This cell type should be off");
  }
};

void verify_dims_value()
{
  test(smtk::mesh::Dims0 == 0, "Dims0 enum value has changed");
  test(smtk::mesh::Dims1 == 1, "Dims1 enum value has changed");
  test(smtk::mesh::Dims2 == 2, "Dims2 enum value has changed");
  test(smtk::mesh::Dims3 == 3, "Dims3 enum value has changed");
}

void verify_CellTypes_usage()
{
  //verify that the empty construct, string constructor and
  //bitwise contstructor all work
  smtk::mesh::testing::TryAllCellEnums(verify_all_off_cell_types());
  smtk::mesh::testing::TryAllCellEnums(verify_all_on_cell_types());

  //verify the string constructors
  smtk::mesh::CellTypes ctypes_only_line(std::string("10"));
  smtk::mesh::CellTypes ctypes_only_2d(std::string("000011100"));

  test(ctypes_only_line[smtk::mesh::Vertex] == false);
  test(ctypes_only_line[smtk::mesh::Line] == true);
  test(ctypes_only_line[smtk::mesh::Triangle] == false);
  test(ctypes_only_line[smtk::mesh::Quad] == false);

  test(ctypes_only_2d[smtk::mesh::Vertex] == false);
  test(ctypes_only_2d[smtk::mesh::Line] == false);
  test(ctypes_only_2d[smtk::mesh::Triangle] == true);
  test(ctypes_only_2d[smtk::mesh::Quad] == true);
  test(ctypes_only_2d[smtk::mesh::Polygon] == true);
  test(ctypes_only_2d[smtk::mesh::Tetrahedron] == false);
  test(ctypes_only_2d[smtk::mesh::Pyramid] == false);
  test(ctypes_only_2d[smtk::mesh::Wedge] == false);
  test(ctypes_only_2d[smtk::mesh::Hexahedron] == false);

  //verify the long long constructors
  smtk::mesh::CellTypes ctypes_only_3d(992);
  smtk::mesh::CellTypes ctypes_only_1d(2);

  test(ctypes_only_3d[smtk::mesh::Vertex] == false);
  test(ctypes_only_3d[smtk::mesh::Line] == false);
  test(ctypes_only_3d[smtk::mesh::Triangle] == false);
  test(ctypes_only_3d[smtk::mesh::Quad] == false);
  test(ctypes_only_3d[smtk::mesh::Polygon] == false);
  test(ctypes_only_3d[smtk::mesh::Tetrahedron] == true);
  test(ctypes_only_3d[smtk::mesh::Pyramid] == true);
  test(ctypes_only_3d[smtk::mesh::Wedge] == true);
  test(ctypes_only_3d[smtk::mesh::Hexahedron] == true);

  test(ctypes_only_1d[smtk::mesh::Vertex] == false);
  test(ctypes_only_1d[smtk::mesh::Line] == true);
  test(ctypes_only_1d[smtk::mesh::Triangle] == false);
  test(ctypes_only_1d[smtk::mesh::Quad] == false);
}

void verify_DimsType_usage()
{
  smtk::mesh::DimensionTypes dtypes_all_off;
  smtk::mesh::DimensionTypes dtypes_all_on(std::string("1111"));
  smtk::mesh::DimensionTypes dtypes_2d_only(4);
  smtk::mesh::DimensionTypes dtypes_3d_only(std::string("1000"));

  test(dtypes_all_off[smtk::mesh::Dims0] == false);
  test(dtypes_all_off[smtk::mesh::Dims1] == false);
  test(dtypes_all_off[smtk::mesh::Dims2] == false);
  test(dtypes_all_off[smtk::mesh::Dims3] == false);

  test(dtypes_all_on[smtk::mesh::Dims0] == true);
  test(dtypes_all_on[smtk::mesh::Dims1] == true);
  test(dtypes_all_on[smtk::mesh::Dims2] == true);
  test(dtypes_all_on[smtk::mesh::Dims3] == true);

  test(dtypes_2d_only[smtk::mesh::Dims0] == false);
  test(dtypes_2d_only[smtk::mesh::Dims1] == false);
  test(dtypes_2d_only[smtk::mesh::Dims2] == true);
  test(dtypes_2d_only[smtk::mesh::Dims3] == false);

  test(dtypes_3d_only[smtk::mesh::Dims0] == false);
  test(dtypes_3d_only[smtk::mesh::Dims1] == false);
  test(dtypes_3d_only[smtk::mesh::Dims2] == false);
  test(dtypes_3d_only[smtk::mesh::Dims3] == true);
}
}

int UnitTestQueryTypes(int, char** const)
{
  verify_dims_value();
  verify_CellTypes_usage();
  verify_DimsType_usage();

  return 0;
}
