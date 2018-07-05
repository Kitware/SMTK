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

smtkComponentInitMacro(smtk_rgg_session);
smtkPythonInitMacro(export_to_pyarc, smtk.bridge.rgg.export_to_pyarc, true);
