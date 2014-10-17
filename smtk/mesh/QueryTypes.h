//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_QueryTypes_h
#define __smtk_mesh_QueryTypes_h

//Query Types is a convenience header, whose goal is to make it easier
//for users to query a manager

#include "smtk/mesh/CellTypes.h"
#include "smtk/mesh/DimensionTypes.h"

namespace smtk {
namespace mesh {

typedef int TypeSet;
typedef int CellSet;
typedef int PointSet;
typedef int MeshSet;

}
}

#endif