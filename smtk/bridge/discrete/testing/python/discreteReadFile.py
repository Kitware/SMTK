#!/usr/bin/python
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
import smtk.testing
from smtk.simple import *

try:
  import unittest2 as unittest
except ImportError:
  import unittest

class TestDiscreteSession(unittest.TestCase):

  def setUp(self):
    import os, sys
    self.filename = os.path.join(smtk.testing.DATA_DIR, 'cmb', 'test2D.cmb')

    self.mgr = smtk.model.Manager.create()
    sess = self.mgr.createSession('discrete')
    sess.assignDefaultName()
    SetActiveSession(sess)
    self.shouldSave = False

  def testFileTypes(self):
    sref = GetActiveSession()
    print '\n\n%s: type "%s" %s %s' % \
      (sref.name(), sref.session().name(), sref.flagSummary(0), sref.session().sessionId())
    print '  Site: %s' % (sref.site() or 'local')
    for eng in sref.engines():
      print '  Engine %s filetypes:\n    %s' % \
        (eng, '\n    '.join(sref.fileTypes(eng)))
    print 'Operators:\n  '
    print '\n  '.join(sref.operatorNames())
    print '\n'

  def testRead(self):
    mod = smtk.model.Model(Read(self.filename)[0])

    print '\nFree cells:\n  %s' % '\n  '.join([x.name() for x in mod.cells()])
    print '\nGroups:\n  %s\n' % '\n  '.join([x.name() for x in mod.groups()])
    if len(mod.cells()) != 4:
      print smtk.io.ExportJSON.fromModelManager(mgr)
    self.assertEqual(len(mod.cells()), 4, 'Expected 4 free cells')

    if self.shouldSave:
      out = file('test2d.json', 'w')
      print >>out, smtk.io.ExportJSON.fromModelManager(mgr)
      out.close()

if __name__ == '__main__':
  smtk.testing.process_arguments()
  unittest.main()
