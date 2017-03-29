//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/AutoInit.h"
#include "smtk/Options.h"

#ifdef SMTK_ENABLE_MESH_SESSION
// If mesh-session is included in the build, ensure that it is loaded
// (and thus registered with the model manager).
smtkComponentInitMacro(smtk_mesh_session);
#endif // SMTK_ENABLE_MESH_SESSION
