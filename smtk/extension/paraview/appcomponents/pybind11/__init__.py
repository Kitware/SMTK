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

from ._smtkPybindPQComponents import *


def importPythonOperation(moduleName, operationName):
    """Register the named operation (from the named module) with the
    active server's operation manager.

    Typically, you will call this from inside a python module like so:
    ```python
      import smtk.operation
      class CustomOperation(smtk.operation.Operation):
          # ...

      if __name__ != '__main__':
          try:
              import smtk.extension.paraview.appcomponents as pv
              pv.importPythonOperation(__name__, 'CustomOperation')
          finally:
              loaded = True # Do-nothing statement
    ```

    You can then use the Tools→Manage Plugins… menu to load the python
    file containing the above as a plugin. Set it to auto-load and the
    operation will always be registered when you run modelbuilder.
    """
    pqSMTKBehavior.importPythonOperationsForModule(moduleName, operationName)
