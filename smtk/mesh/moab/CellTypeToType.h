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


#ifndef __smtk_mesh_moab_CellTypeToType_h
#define __smtk_mesh_moab_CellTypeToType_h

#include "smtk/mesh/moab/Types.h"
#include "smtk/mesh/CellTypes.h"

namespace smtk {
namespace mesh {
namespace moab {

inline
smtk::mesh::CellType moabToSMTKCell(smtk::mesh::moab::EntityType t)
  {
  smtk::mesh::CellType ctype = smtk::mesh::CellType_MAX;
  switch (t)
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

inline
smtk::mesh::moab::EntityType smtkToMOABCell(smtk::mesh::CellType t)
  {
  smtk::mesh::moab::EntityType ctype = ::moab::MBMAXTYPE;
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

#endif // __smtk_mesh_moab_CellTypeToType_h