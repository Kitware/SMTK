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

mgr = smtk.model.Manager.create()
sref = mgr.createSession('discrete', smtk.model.SessionRef())
sess = sref.session() # smtk.model.Manager.createSession('cgm', smtk.model.SessionRef())
sref.assignDefaultName()
print '\n\n%s: type "%s" %s %s' % \
  (sref.name(), sess.name(), sref.flagSummary(0), sess.sessionId())
print '  Site: %s' % (sref.site() or 'local')
for eng in sref.engines():
  print '  Engine %s filetypes:\n    %s' % \
    (eng, '\n    '.join(sref.fileTypes(eng)))
print 'Operators:\n  '
print '\n  '.join(sref.operatorNames())
print '\n'

rdr = sref.op('read')
rdr.findAsFile('filename').setValue(os.path.join(sys.argv[1], 'cmb', 'test2D.cmb'))
res = rdr.operate()
mod = smtk.model.Model(res.findModelEntity('entities').value(0))

print '\nFree cells:\n  %s' % '\n  '.join([x.name() for x in mod.cells()])
print '\nGroups:\n  %s\n' % '\n  '.join([x.name() for x in mod.groups()])
if len(mod.cells()) != 4:
  print smtk.io.ExportJSON.fromModelManager(mgr)
  raise Exception, 'Wrong number of free cells'
