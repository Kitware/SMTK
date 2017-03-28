//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/Options.h"
#include "smtk/AutoInit.h"

#ifdef SMTK_ENABLE_MULTISCALE_SESSION
// If multiscale-session is included in the build, ensure that it is loaded
// (and thus registered with the model manager).
smtkComponentInitMacro(smtk_multiscale_session);
#endif // SMTK_ENABLE_MULTISCALE_SESSION
