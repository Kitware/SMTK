//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_CellTypes_h
#define __smtk_mesh_CellTypes_h

#include "smtk/CoreExports.h"

#include <bitset>
#include <string>

namespace smtk {
  namespace mesh {

/**\brief Enum types used for querying the interface.
  *
  * When changing this enum, be sure to update verticesPerCell()
  * and cellTypeName()!
  */
enum CellType
{
  Vertex        = 0,
  Line          = 1,
  Triangle      = 2,
  Quad          = 3,
  Polygon       = 4,
  Tetrahedron   = 5,
  Pyramid       = 6,
  Wedge         = 7,
  Hexahedron    = 8,
  CellType_MAX  = 9
};

SMTKCORE_EXPORT int verticesPerCell(CellType ctype);
SMTKCORE_EXPORT std::string cellTypeSummary(CellType ctype, int flag = 0);

//Need a basic blitter for cell queries, mainly used by
//TypeSet
typedef std::bitset<CellType_MAX> CellTypes;

//concrete types that you can cast the enum to by using CellTraits.h
//useful when you need to template code based on the cell type
struct CellVertex {       static const CellType CellEnum = Vertex; };
struct CellLine {         static const CellType CellEnum = Line; };
struct CellTriangle {     static const CellType CellEnum = Triangle; };
struct CellQuad {         static const CellType CellEnum = Quad; };
struct CellPolygon {      static const CellType CellEnum = Polygon; };
struct CellTetrahedron {  static const CellType CellEnum = Tetrahedron; };
struct CellPyramid {      static const CellType CellEnum = Pyramid; };
struct CellWedge {        static const CellType CellEnum = Wedge; };
struct CellHexahedron {   static const CellType CellEnum = Hexahedron; };

  } // namespace mesh
} // namespace smtk

#endif //__smtk_mesh_CellTypes_h
