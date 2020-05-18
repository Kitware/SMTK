//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/core/QueryTypes.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

struct verify_all_off_cell_types
{
  smtk::mesh::CellTypes ctypes;
  //initialize the cellTypes for this struct to be all off
  verify_all_off_cell_types() = default;

  void operator()(smtk::mesh::CellType cellEnum)
  { //this operator will be called for each CellType, so verify that
    //the struct 's ctypes bit is turned off for the given type
    test(!ctypes[cellEnum], "This cell type should be off");
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
    test(ctypes[cellEnum], "This cell type should be off");
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

  test(!ctypes_only_line[smtk::mesh::Vertex]);
  test(ctypes_only_line[smtk::mesh::Line]);
  test(!ctypes_only_line[smtk::mesh::Triangle]);
  test(!ctypes_only_line[smtk::mesh::Quad]);

  test(!ctypes_only_2d[smtk::mesh::Vertex]);
  test(!ctypes_only_2d[smtk::mesh::Line]);
  test(ctypes_only_2d[smtk::mesh::Triangle]);
  test(ctypes_only_2d[smtk::mesh::Quad]);
  test(ctypes_only_2d[smtk::mesh::Polygon]);
  test(!ctypes_only_2d[smtk::mesh::Tetrahedron]);
  test(!ctypes_only_2d[smtk::mesh::Pyramid]);
  test(!ctypes_only_2d[smtk::mesh::Wedge]);
  test(!ctypes_only_2d[smtk::mesh::Hexahedron]);

  //verify the long long constructors
  smtk::mesh::CellTypes ctypes_only_3d(992);
  smtk::mesh::CellTypes ctypes_only_1d(2);

  test(!ctypes_only_3d[smtk::mesh::Vertex]);
  test(!ctypes_only_3d[smtk::mesh::Line]);
  test(!ctypes_only_3d[smtk::mesh::Triangle]);
  test(!ctypes_only_3d[smtk::mesh::Quad]);
  test(!ctypes_only_3d[smtk::mesh::Polygon]);
  test(ctypes_only_3d[smtk::mesh::Tetrahedron]);
  test(ctypes_only_3d[smtk::mesh::Pyramid]);
  test(ctypes_only_3d[smtk::mesh::Wedge]);
  test(ctypes_only_3d[smtk::mesh::Hexahedron]);

  test(!ctypes_only_1d[smtk::mesh::Vertex]);
  test(ctypes_only_1d[smtk::mesh::Line]);
  test(!ctypes_only_1d[smtk::mesh::Triangle]);
  test(!ctypes_only_1d[smtk::mesh::Quad]);
}

void verify_DimsType_usage()
{
  smtk::mesh::DimensionTypes dtypes_all_off;
  smtk::mesh::DimensionTypes dtypes_all_on(std::string("1111"));
  smtk::mesh::DimensionTypes dtypes_2d_only(4);
  smtk::mesh::DimensionTypes dtypes_3d_only(std::string("1000"));

  test(!dtypes_all_off[smtk::mesh::Dims0]);
  test(!dtypes_all_off[smtk::mesh::Dims1]);
  test(!dtypes_all_off[smtk::mesh::Dims2]);
  test(!dtypes_all_off[smtk::mesh::Dims3]);

  test(dtypes_all_on[smtk::mesh::Dims0]);
  test(dtypes_all_on[smtk::mesh::Dims1]);
  test(dtypes_all_on[smtk::mesh::Dims2]);
  test(dtypes_all_on[smtk::mesh::Dims3]);

  test(!dtypes_2d_only[smtk::mesh::Dims0]);
  test(!dtypes_2d_only[smtk::mesh::Dims1]);
  test(dtypes_2d_only[smtk::mesh::Dims2]);
  test(!dtypes_2d_only[smtk::mesh::Dims3]);

  test(!dtypes_3d_only[smtk::mesh::Dims0]);
  test(!dtypes_3d_only[smtk::mesh::Dims1]);
  test(!dtypes_3d_only[smtk::mesh::Dims2]);
  test(dtypes_3d_only[smtk::mesh::Dims3]);
}
} // namespace

int UnitTestQueryTypes(int /*unused*/, char** const /*unused*/)
{
  verify_dims_value();
  verify_CellTypes_usage();
  verify_DimsType_usage();

  return 0;
}
