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

// Ensure that the openfoam session is loaded (and thus registered with the
// model manager).
smtkComponentInitMacro(smtk_openfoam_session);

// Also add python operators.
smtkPythonInitMacro(add_obstacle, smtk.bridge.openfoam.add_obstacle, true);
smtkPythonInitMacro(create_wind_tunnel, smtk.bridge.openfoam.create_wind_tunnel, true);
smtkPythonInitMacro(set_main_controls, smtk.bridge.openfoam.set_main_controls, true);
smtkPythonInitMacro(set_working_directory, smtk.bridge.openfoam.set_working_directory, true);
