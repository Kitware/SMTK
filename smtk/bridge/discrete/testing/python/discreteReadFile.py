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

  def resetTestFiles(self):
    self.filesToTest = []

  def addExternalFile(self, pathStr, numCells, numGroups):
    self.filesToTest += [{'filename':pathStr, 'numCells':numCells, 'numGroups':numGroups}]

  def addTestFile(self, pathList, numCells, numGroups):
    self.addExternalFile(
      os.path.join(*([smtk.testing.DATA_DIR,] + pathList)),
      numCells, numGroups)

  def setUp(self):
    import os, sys
    self.resetTestFiles()
    self.addTestFile(['cmb', 'test2D.cmb'], 4, 0)
    self.addTestFile(['cmb', 'SimpleBox.cmb'], 1, 2)
    self.addTestFile(['cmb', 'smooth_surface.cmb'], 6, 0)

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

  def verifyRead(self, filename, numCells, numGroups):
    print '\n\nFile: {fname}'.format(fname=filename)

    mod = smtk.model.Model(Read(filename)[0])

    print '\nFree cells:\n  %s' % '\n  '.join([x.name() for x in mod.cells()])
    print '\nGroups:\n  %s\n' % '\n  '.join([x.name() for x in mod.groups()])
    if (numCells >= 0 and len(mod.cells()) != numCells) or (numGroups >= 0 and len(mod.groups()) != numGroups):
      print smtk.io.ExportJSON.fromModelManager(self.mgr)

    if numCells >= 0:
      self.assertEqual(len(mod.cells()), numCells, 'Expected {nc} free cells'.format(nc=numCells))
    if numGroups >= 0:
      self.assertEqual(len(mod.groups()), numGroups, 'Expected {ng} groups'.format(ng=numGroups))

  def testRead(self):
    for test in self.filesToTest:
      self.verifyRead(test['filename'], test['numCells'], test['numGroups'])

    if self.shouldSave:
      out = file('test.json', 'w')
      print >>out, smtk.io.ExportJSON.fromModelManager(self.mgr)
      out.close()

if __name__ == '__main__':
  smtk.testing.process_arguments()
  unittest.main()
