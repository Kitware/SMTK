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
sess = mgr.createSession('discrete')
brg = sess.bridge() # smtk.model.Manager.createBridge('cgm')
sess.assignDefaultName()
print '\n\n%s: type "%s" %s %s' % \
  (sess.name(), brg.name(), sess.flagSummary(0), brg.sessionId())
print '  Site: %s' % (sess.site() or 'local')
for eng in sess.engines():
  print '  Engine %s filetypes:\n    %s' % \
    (eng, '\n    '.join(sess.fileTypes(eng)))
print 'Operators:\n  '
print '\n  '.join(sess.operatorNames())
print '\n'

rdr = sess.op('read')
rdr.findAsFile('filename').setValue(os.path.join(sys.argv[1], 'cmb', 'test2D.cmb'))
res = rdr.operate()
mod = smtk.model.ModelEntity(res.findModelEntity('entities').value(0))

print '\nFree cells:\n  %s' % '\n  '.join([x.name() for x in mod.cells()])
print '\nGroups:\n  %s\n' % '\n  '.join([x.name() for x in mod.groups()])
if len(mod.cells()) != 4:
  print smtk.io.ExportJSON.fromModel(mgr)
  raise Exception, 'Wrong number of free cells'
