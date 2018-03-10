#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================

""" set_working_directory.py:

OpenFOAM's workflow operates around a top-level working directory. This operator
sets that directory for subsequent OpenFOAM actions.
"""

import os
import smtk
import smtk.bridge.openfoam


@smtk.model.operator("set working directory", smtk.bridge.openfoam.Session)
class set_working_directory(smtk.model.Operator):

    def __init__(self):
        smtk.model.Operator.__init__(self)

    def name(self):
        return "set working directory"

    def className(self):
        return self.__name__

    def operateInternal(self):

        # Access the working directory
        workingDirectory = self.specification().find(
            'working directory').value(0)

        # Set the working directory in the session
        self.activeSession().setWorkingDirectory(workingDirectory)

        # Exit with success
        return self.createResult(smtk.model.OPERATION_SUCCEEDED)
