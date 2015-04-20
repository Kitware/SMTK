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

namespace smtk {
  namespace mesh {

/**\brief Return the number of vertices that define a cell of this type (or -1).
  *
  */
int verticesPerCell(CellType ctype)
{
  static int verticesByType[CellType_MAX] = {1, 2, 3, 4, -1, 4, 5, 6, 8};
  return ctype >= 0 && ctype <= CellType_MAX ? verticesByType[ctype] : -1;
}

  } // namespace mesh
} // namespace smtk
