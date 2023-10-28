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

import sys
import smtk
import smtk.common

loaded, skipped = smtk.loadPlugins()

expectPlugins = False
try:
    from paraview.simple import LoadPlugin
    expectPlugins = True
except:
    pass

error = False
if expectPlugins:
    print('Skipped %d plugins.' % len(skipped))
    for plugin in skipped:
        print('  %s' % plugin)
    print('Loaded %d plugins.' % len(loaded))
    for plugin in loaded:
        print('  %s' % plugin)
    if len(loaded) == 0:
        print('ERROR: Expected to load at least one plugin.')
        error = True
    elif len(skipped) > 0:
        print('ERROR: Expected all plugins to load successfully.')
        error = True

# Now create an application context and verify an operation and resource
# manager were added to it.

appContext = smtk.applicationContext()
if not appContext:
    print('ERROR: Expected to find or create an smtk.common.Managers object.')
    error = True

operMgr = appContext.get('smtk.operation.Manager')
rsrcMgr = appContext.get('smtk.resource.Manager')
if not operMgr or not rsrcMgr:
    print('ERROR: Expected to find initialized resource and operation managers.')
    error = True

sys.exit(1 if error else 0)
