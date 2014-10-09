//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_DimensionTypes_h
#define __smtk_mesh_DimensionTypes_h


#include <bitset>

namespace smtk {
namespace mesh {


enum DimensionType
{
  Dims0 = 0,
  Dims1 = 1,
  Dims2 = 2,
  Dims3 = 3,
  DimensionType_MAX = 4
};

//Need a basic blitter for dims queries
typedef std::bitset<DimensionType_MAX> DimensionTypes;

}
}

#endif //__smtk_mesh_DimensionTypes_h