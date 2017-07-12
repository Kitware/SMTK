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
#include "smtk/PythonAutoInit.h"

// Ensure that the multiscale session is loaded (and thus registered with the
// model manager).
smtkComponentInitMacro(smtk_multiscale_session);

// Also add python operators.
smtkPythonInitMacro(import_from_deform, smtk.bridge.multiscale.import_from_deform);
