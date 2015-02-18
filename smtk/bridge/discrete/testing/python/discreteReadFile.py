#!/usr/bin/python
import sys
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
from smtk.simple import *

mgr = smtk.model.Manager.create()
sref = mgr.createSession('discrete', smtk.model.SessionRef())
SetActiveSession(sref)
sref.assignDefaultName()

print '\n\n%s: type "%s" %s %s' % \
  (sref.name(), sref.session().name(), sref.flagSummary(0), sref.session().sessionId())
print '  Site: %s' % (sref.site() or 'local')
for eng in sref.engines():
  print '  Engine %s filetypes:\n    %s' % \
    (eng, '\n    '.join(sref.fileTypes(eng)))
print 'Operators:\n  '
print '\n  '.join(sref.operatorNames())
print '\n'

mod = smtk.model.Model(
    Read(os.path.join(sys.argv[1], 'cmb', 'test2D.cmb'))[0])

print '\nFree cells:\n  %s' % '\n  '.join([x.name() for x in mod.cells()])
print '\nGroups:\n  %s\n' % '\n  '.join([x.name() for x in mod.groups()])
if len(mod.cells()) != 4:
  print smtk.io.ExportJSON.fromModelManager(mgr)
  raise Exception, 'Wrong number of free cells'

out = file('test2d.json', 'w')
print >>out, smtk.io.ExportJSON.fromModelManager(mgr)
out.close()
