//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/CellTraits.h"
#include "smtk/mesh/CellTypes.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/common/CompilerInformation.h"

#if defined(SMTK_CLANG) || (defined(__GNUC__) && (__GNUC__ > 5 && __GNUC_MINOR__ > 0))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-compare"
#endif

namespace
{

struct verify_cell_attributes
{
  template <class CellType>
  void operator()(CellType type)
  {
    (void)type;
    smtk::mesh::CellTraits<CellType> traits;
    this->verify(traits);
  }

  void operator()(smtk::mesh::CellType cellEnum)
  {
    //this is the issue transferring from an enum type to a concrete
    //type by using a macroed switch statement
    switch (cellEnum)
    {
      smtkMeshCellEnumToTypeMacro(this->verify(CellTraits()));
      case smtk::mesh::CellType_MAX:
        break;
    }
  }

  void verify(smtk::mesh::CellTraits<smtk::mesh::CellHexahedron> traits)
  {
    test(traits.NUM_VERTICES == 8, "CellHexahedron Trait reports wrong number of points");
    test(traits.fixedPointSize() == smtk::mesh::CellFixedPointNumberTag::Type,
      "CellHexahedron Traits reports fixedPointSize() incorrectly");
    test(traits.dimension() == 3, "CellHexahedron Trait reports wrong dimension");

    //now verify the enum logic is correct
    typedef smtk::mesh::CellTraits<smtk::mesh::CellHexahedron> TraitsType;
    test(TraitsType::CellType::CellEnum == smtk::mesh::CellHexahedron::CellEnum,
      "CellEnum from the traits isn't correct");
  }
  void verify(smtk::mesh::CellTraits<smtk::mesh::CellLine> traits)
  {
    test(traits.NUM_VERTICES == 2, "CellLine Trait reports wrong number of points");
    test(traits.fixedPointSize() == smtk::mesh::CellFixedPointNumberTag::Type,
      "CellLine Traits reports fixedPointSize() incorrectly");
    test(traits.dimension() == 1, "CellLine Trait reports wrong dimension");

    //now verify the enum logic is correct
    typedef smtk::mesh::CellTraits<smtk::mesh::CellLine> TraitsType;
    test(TraitsType::CellType::CellEnum == smtk::mesh::CellLine::CellEnum,
      "CellEnum from the traits isn't correct");
  }
  void verify(smtk::mesh::CellTraits<smtk::mesh::CellPolygon> traits)
  {
    test(traits.NUM_VERTICES == -1, "CellPolygon Trait reports wrong number of points");
    test(traits.fixedPointSize() != smtk::mesh::CellFixedPointNumberTag::Type,
      "CellPolygon Traits reports fixedPointSize() incorrectly");
    test(traits.dimension() == 2, "CellPolygon Trait reports wrong dimension");

    //now verify the enum logic is correct
    typedef smtk::mesh::CellTraits<smtk::mesh::CellPolygon> TraitsType;
    test(TraitsType::CellType::CellEnum == smtk::mesh::CellPolygon::CellEnum,
      "CellEnum from the traits isn't correct");
  }
  void verify(smtk::mesh::CellTraits<smtk::mesh::CellPyramid> traits)
  {
    test(traits.NUM_VERTICES == 5, "CellPyramid Trait reports wrong number of points");
    test(traits.fixedPointSize() == smtk::mesh::CellFixedPointNumberTag::Type,
      "CellPyramid Traits reports fixedPointSize() incorrectly");
    test(traits.dimension() == 3, "CellPyramid Trait reports wrong dimension");

    //now verify the enum logic is correct
    typedef smtk::mesh::CellTraits<smtk::mesh::CellPyramid> TraitsType;
    test(TraitsType::CellType::CellEnum == smtk::mesh::CellPyramid::CellEnum,
      "CellEnum from the traits isn't correct");
  }
  void verify(smtk::mesh::CellTraits<smtk::mesh::CellQuad> traits)
  {
    test(traits.NUM_VERTICES == 4, "CellQuad Trait reports wrong number of points");
    test(traits.fixedPointSize() == smtk::mesh::CellFixedPointNumberTag::Type,
      "CellQuad Traits reports fixedPointSize() incorrectly");
    test(traits.dimension() == 2, "CellQuad Trait reports wrong dimension");

    //now verify the enum logic is correct
    typedef smtk::mesh::CellTraits<smtk::mesh::CellQuad> TraitsType;
    test(TraitsType::CellType::CellEnum == smtk::mesh::CellQuad::CellEnum,
      "CellEnum from the traits isn't correct");
  }
  void verify(smtk::mesh::CellTraits<smtk::mesh::CellTetrahedron> traits)
  {
    test(traits.NUM_VERTICES == 4, "CellTetrahedron Trait reports wrong number of points");
    test(traits.fixedPointSize() == smtk::mesh::CellFixedPointNumberTag::Type,
      "CellTetrahedron Traits reports fixedPointSize() incorrectly");
    test(traits.dimension() == 3, "CellTetrahedron Trait reports wrong dimension");

    //now verify the enum logic is correct
    typedef smtk::mesh::CellTraits<smtk::mesh::CellTetrahedron> TraitsType;
    test(TraitsType::CellType::CellEnum == smtk::mesh::CellTetrahedron::CellEnum,
      "CellEnum from the traits isn't correct");
  }
  void verify(smtk::mesh::CellTraits<smtk::mesh::CellTriangle> traits)
  {
    test(traits.NUM_VERTICES == 3, "CellTriangle Trait reports wrong number of points");
    test(traits.fixedPointSize() == smtk::mesh::CellFixedPointNumberTag::Type,
      "CellTriangle Traits reports fixedPointSize() incorrectly");
    test(traits.dimension() == 2, "CellTriangle Trait reports wrong dimension");

    //now verify the enum logic is correct
    typedef smtk::mesh::CellTraits<smtk::mesh::CellTriangle> TraitsType;
    test(TraitsType::CellType::CellEnum == smtk::mesh::CellTriangle::CellEnum,
      "CellEnum from the traits isn't correct");
  }
  void verify(smtk::mesh::CellTraits<smtk::mesh::CellVertex> traits)
  {
    test(traits.NUM_VERTICES == 1, "CellVertex Trait reports wrong number of points");
    test(traits.fixedPointSize() == smtk::mesh::CellFixedPointNumberTag::Type,
      "CellVertex Traits reports fixedPointSize() incorrectly");
    test(traits.dimension() == 0, "CellVertex Trait reports wrong dimension");

    //now verify the enum logic is correct
    typedef smtk::mesh::CellTraits<smtk::mesh::CellVertex> TraitsType;
    test(TraitsType::CellType::CellEnum == smtk::mesh::CellVertex::CellEnum,
      "CellEnum from the traits isn't correct");
  }
  void verify(smtk::mesh::CellTraits<smtk::mesh::CellWedge> traits)
  {
    test(traits.NUM_VERTICES == 6, "CellWedge Trait reports wrong number of points");
    test(traits.fixedPointSize() == smtk::mesh::CellFixedPointNumberTag::Type,
      "CellWedge Traits reports fixedPointSize() incorrectly");
    test(traits.dimension() == 3, "CellWedge Trait reports wrong dimension");

    //now verify the enum logic is correct
    typedef smtk::mesh::CellTraits<smtk::mesh::CellWedge> TraitsType;
    test(TraitsType::CellType::CellEnum == smtk::mesh::CellWedge::CellEnum,
      "CellEnum from the traits isn't correct");
  }
};

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
}

int UnitTestCellTypes(int, char** const)
{
  std::cout << "verify_cell_attributes" << std::endl;
  smtk::mesh::testing::TryAllCells(verify_cell_attributes());
  smtk::mesh::testing::TryAllCellEnums(verify_cell_attributes());

  return 0;
}
