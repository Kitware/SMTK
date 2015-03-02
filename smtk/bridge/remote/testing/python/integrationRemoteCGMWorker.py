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
import unittest
import smtk
import smtk.testing
from smtk.simple import *
import os, sys

class IntegrationRemoteCGMWorker(unittest.TestCase):
    """Start a remote model worker session and run
       successive operations on entities to ensure that
       the session stays alive and transmits the proper
       entity information."""

    def setUp(self):
        self.mgr = smtk.model.Manager.create()
        self.conn = smtk.bridge.remote.RemusConnection.create()
        self.conn.addSearchDir(smtk.testing.WORKER_DIR)
        print 'Session names\n  ' + '\n  '.join(self.conn.sessionTypeNames())
        cgmWorkerName = 'smtk::model[cgm{OpenCascade}@ctest]'
        if cgmWorkerName not in self.conn.sessionTypeNames():
          print 'Skipping test due to missing worker.'
          self.skipTest('Remote CGM session unavailable.')
        sess = self.mgr.createSession(cgmWorkerName)
        print 'Created session'
        SetActiveSession(sess)

    def testGeometryCreation(self):
        brick = CreateBrick()
        print 'Created brick %s' % brick.name()
        tbrick = Translate(brick, [5,0,0])
        self.assertEqual(
            len(tbrick), 1, 'Expected 1 model result')
        print 'Translated brick %s' % tbrick[0].name()
        filename = os.path.join(smtk.testing.TEMP_DIR, 'tbrick.brep')
        status = Write(filename, tbrick)
        self.assertEqual(
            status, smtk.model.OPERATION_SUCCEEDED,
            'Failed to write generated model.')

    def testGeometryModification(self):
        filename = os.path.join(smtk.testing.DATA_DIR, 'cgm', 'pyramid.brep')
        pyramid = Read(filename)[0]
        brick = CreateBrick()
        brick = Translate(brick, [0,0,-1])[0]
        result = Union([brick,pyramid])
        filename = os.path.join(smtk.testing.TEMP_DIR, 'house.brep')
        status = Write(filename, [result,])

if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
