//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/EntityType.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include "smtk/mesh/moab/CellTypeToType.h"

namespace smtk {
namespace mesh {
namespace moab {

smtk::mesh::CellType moabToSMTKCell(int t)
  {
  ::moab::EntityType et = static_cast< ::moab::EntityType >(t);
  smtk::mesh::CellType ctype = smtk::mesh::CellType_MAX;
  switch (et)
    {
    case ::moab::MBVERTEX:
      ctype = smtk::mesh::Vertex;
      break;
    case ::moab::MBEDGE:
      ctype = smtk::mesh::Line;
      break;
    case ::moab::MBTRI:
      ctype = smtk::mesh::Triangle;
      break;
    case ::moab::MBQUAD:
      ctype = smtk::mesh::Quad;
      break;
    case ::moab::MBPOLYGON:
      ctype = smtk::mesh::Polygon;
      break;
    case ::moab::MBTET:
      ctype = smtk::mesh::Tetrahedron;
      break;
    case ::moab::MBPYRAMID:
      ctype = smtk::mesh::Pyramid;
      break;
    case ::moab::MBPRISM:
      ctype = smtk::mesh::Wedge;
      break;
    case ::moab::MBHEX:
      ctype = smtk::mesh::Hexahedron;
      break;
    default:
      ctype = smtk::mesh::CellType_MAX;
      break;
    }
  return ctype;
  }

int smtkToMOABCell(smtk::mesh::CellType t)
  {
  ::moab::EntityType ctype = ::moab::MBMAXTYPE;
  switch (t)
    {
    case smtk::mesh::Vertex:
      ctype = ::moab::MBVERTEX;
      break;
    case smtk::mesh::Line:
      ctype = ::moab::MBEDGE;
      break;
    case smtk::mesh::Triangle:
      ctype = ::moab::MBTRI;
      break;
    case smtk::mesh::Quad:
      ctype = ::moab::MBQUAD;
      break;
    case smtk::mesh::Polygon:
      ctype = ::moab::MBPOLYGON;
      break;
    case smtk::mesh::Tetrahedron:
      ctype = ::moab::MBTET;
      break;
    case smtk::mesh::Pyramid:
      ctype = ::moab::MBPYRAMID;
      break;
    case smtk::mesh::Wedge:
      ctype = ::moab::MBPRISM;
      break;
    case smtk::mesh::Hexahedron:
      ctype = ::moab::MBHEX;
      break;
    default:
      ctype = ::moab::MBMAXTYPE;
      break;
    }
  return ctype;
  }

}
}
} //namespace smtk::mesh::moab
