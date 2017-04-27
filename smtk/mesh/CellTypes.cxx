//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/CellTypes.h"

namespace smtk
{
namespace mesh
{

/**\brief Return the number of vertices that define a cell of this type (or -1).
  *
  */
int verticesPerCell(CellType ctype)
{
  static int verticesByType[CellType_MAX] = { 1, 2, 3, 4, -1, 4, 5, 6, 8 };
  return ctype >= 0 && ctype <= CellType_MAX ? verticesByType[ctype] : -1;
}

/**\brief Return the name of the mesh cell type.
  *
  * If the \a flag value is non-zero, return the plural form of the name.
  */
std::string cellTypeSummary(CellType ctype, int flag)
{
  static const char* cellTypeNamesSingular[CellType_MAX + 1] = {
    "vertex", "line", "triangle", "quad", "polygon", "tetrahedron", "pyramid", "wedge",
    "hexahedron",
    "invalid" // CellType_MAX
  };
  static const char* cellTypeNamesPlural[CellType_MAX + 1] = {
    "vertices", "lines", "triangles", "quads", "polygons", "tetrahedra", "pyramids", "wedges",
    "hexahedra",
    "invalid" // CellType_MAX
  };
  return ctype >= 0 && ctype <= CellType_MAX
    ? (flag ? cellTypeNamesPlural[ctype] : cellTypeNamesSingular[ctype])
    : cellTypeNamesSingular[CellType_MAX];
}

} // namespace mesh
} // namespace smtk
