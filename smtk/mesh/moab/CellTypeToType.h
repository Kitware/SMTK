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

#ifndef smtk_mesh_moab_CellTypeToType_h
#define smtk_mesh_moab_CellTypeToType_h

#include "smtk/mesh/core/CellTypes.h"

namespace smtk
{
namespace mesh
{
namespace moab
{

//these aren't exported as they are private functions that only
//smtk::mesh should call

smtk::mesh::CellType moabToSMTKCell(int t);

int smtkToMOABCell(smtk::mesh::CellType t);
} // namespace moab
} // namespace mesh
} // namespace smtk

#endif // smtk_mesh_moab_CellTypeToType_h
