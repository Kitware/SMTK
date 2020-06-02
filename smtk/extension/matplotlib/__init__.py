# =============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
# =============================================================================

from . import render_mesh

import sys
import inspect
import smtk
import smtk.operation


def registerOperations(operationManager):
    for name, obj in inspect.getmembers(render_mesh):
        if inspect.isclass(obj) and issubclass(obj, smtk.operation.Operation):
            operationManager.registerOperation(
                render_mesh.__name__, obj.__name__)
