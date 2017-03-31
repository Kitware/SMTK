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
"""
Try running a "universal" operator (close model) on imported models.
"""

import os
import sys
import smtk
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.bridge
    import smtk.io
    import smtk.model
from smtk.simple import *
import smtk.testing
from uuid import uuid4
import unittest


class TestModelCloseModelOp(unittest.TestCase):

    def loadThenCloseSessionModel(self, sessionname, filename):
        actMgr = smtk.model.Manager.create()
        actSession = actMgr.createSession(sessionname, smtk.model.SessionRef())
        SetActiveSession(actSession)

        models = None
        print 'Reading {fname} into {sname}'.format(fname=filename, sname=sessionname)
        # The 'native' session does not have a "read" op
        if sessionname == 'native':
            json = None
            with open(filename, 'r') as f:
                json = f.read()

            self.assertTrue(not json == None, 'Unable to load input file')
            self.assertTrue(smtk.io.LoadJSON.intoModelManager(
                json, actMgr), 'Unable to parse JSON input file')

            actMgr.assignDefaultNames()
            models = actMgr.findEntitiesOfType(
                int(smtk.model.MODEL_ENTITY), True)
            # Assign imported models to current session so they have operators
            [smtk.model.Model(x).setSession(actSession) for x in models]
        else:
            models = Read(filename)

        print 'Closing %d models.' % len(models)

        result = CloseModel(models)
        self.assertEqual(
            result.findInt('outcome').value(0),
            smtk.model.OPERATION_SUCCEEDED,
            'close model operator failed')
        remModels = GetVectorValue(result.findModelEntity('expunged'))

        print '%d models closed.' % len(remModels)
        self.assertEqual(len(models), len(remModels),
                         'Not all models marked as removed')

    def testCloseModelOp(self):

        status = 0
        if smtk.testing.DATA_DIR != '':
            session_files = {
                'native': ['model', '2d', 'smtk', 'pyramid.json'],
                'discrete': ['model', '2d', 'cmb', 'test2D.cmb'],
                'exodus': ['model', '3d', 'exodus', 'disk_out_ref.ex2'],
                'cgm': ['model', '3d', 'solidmodel', 'occ', 'pyramid.brep']
            }

            for (session_type, path) in session_files.items():
                if session_type == 'native' or session_type in dir(smtk.bridge):
                    filename = os.path.join(
                        *([smtk.testing.DATA_DIR, ] + path))
                    print 'Testing load and close of %s' % filename
                    if self.loadThenCloseSessionModel(session_type, filename) == False:
                        status = 1

        print 'Done'


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
