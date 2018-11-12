//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME rggNucPartDefinition - Definitions for rgg parts
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_session_rgg_qt_rggNucPartDefinition_h
#define __smtk_session_rgg_qt_rggNucPartDefinition_h

#include <assert.h>
#include <sstream>
#include <vector>

enum enumNucPartsType
{
  CMBNUC_ASSY_BASEOBJ = 0,
  CMBNUC_ASSEMBLY = 1,
  CMBNUC_ASSEMBLY_LINK = 2,
  CMBNUC_CORE,
  CMBNUC_ASSY_DUCTCELL,
  CMBNUC_ASSY_PINCELL
};

enum rggGeometryType
{
  RECTILINEAR = 0x0140,
  HEXAGONAL = 0x0241,
};

enum rggGeometryControls
{
  FLAT = 0x0001,
  VERTEX = 0x0002,
  JUST_HEX_SUBTYPE = 0x00FF,
  ANGLE_60 = 0x00100,
  ANGLE_30 = 0x00200,
  ANGLE_360 = 0x00400,
  JUST_ANGLE = 0x0FF00,
};

#endif
