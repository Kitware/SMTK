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
import os
import sys
import smtk
import smtk.session.cgm

if 'cgm' not in smtk.model.Manager.sessionTypeNames():
    print('ERROR: cgm not available.')

    # Print a subset of environment variables to help debugging
    debugEnvNames = (
        'PYTHONPATH', 'DYLD_LIBRARY_PATH', 'DYLD_FALLBACK_LIBRARY_PATH',
        'LD_LIBRARY_PATH', 'PATH', 'LD_PRELOAD'
    )
    print({x: y for x, y in os.environ.items() if x in debugEnvNames})
    sys.exit(1)

ddir = sys.argv[-1]

mgr = smtk.model.Manager.create()
brg = smtk.model.Manager.createSessionOfType('cgm')
mgr.registerSession(brg)

readop = brg.op('read')
readop.findAsFile('filename').setValue(
    os.path.join(ddir, 'cgm', '62_shaver1.brep'))
# Note that we could set the file type here:
#    readop.findAsString('filetype').setValue('OCC')
# but there is no need to in this case as it will be
# correctly inferred from the filename.
res = readop.operate()
