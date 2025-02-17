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
"""
Try running a "universal" operator (close model) on imported models.
"""

import os
import sys
import smtk
import smtk.session
import smtk.session.vtk
import smtk.io
import smtk.model
import smtk.testing
from uuid import uuid4
import unittest


class TestModelCloseModelOp(unittest.TestCase):

    def loadThenCloseSessionModel(self, sessionname, filename):
        actMgr = smtk.model.Resource.create()

        models = None
        print('Reading {fname} into {sname}'.format(
            fname=filename, sname=sessionname))
        # The 'native' session does not have a "read" op
        if sessionname == 'native':
            json = None
            with open(filename, 'r') as f:
                json = f.read()

            self.assertTrue(not json == None, 'Unable to load input file')
            self.assertTrue(smtk.model.SessionIOJSON.loadModelRecords(
                json, actMgr), 'Unable to parse JSON input file')

            actMgr.assignDefaultNames()
            models = actMgr.findEntitiesOfType(
                int(smtk.model.MODEL_ENTITY), True)
            # Assign imported models to current session so they have operators
            session = smtk.model.Session.create()
            actSession = smtk.model.SessionRef(actMgr, session)
            [smtk.model.Model(x).setSession(actSession) for x in models]
        elif sessionname == 'vtk':
            readOp = smtk.session.vtk.Import.create()
            readOp.parameters().find('filename').setValue(filename)
            result = readOp.operate()
            self.assertEqual(
                result.find('outcome').value(0),
                int(smtk.operation.Operation.SUCCEEDED),
                'vtk read operation failed')
            models = smtk.model.Resource.CastTo(
                result.find('resourcesCreated').value(0)).findEntitiesOfType(int(smtk.model.MODEL_ENTITY))

        print('Closing %d models.' % len(models))

        closeModelOp = smtk.model.CloseModel.create()
        closeModelOp.parameters().associate(models[0].component())
        result = closeModelOp.operate()
        self.assertEqual(
            result.find('outcome').value(0),
            int(smtk.operation.Operation.SUCCEEDED),
            'close model operator failed')
        return True

    def testCloseModelOp(self):

        status = 0
        if smtk.testing.DATA_DIR != '':
            session_files = {
                'native': ['model', '2d', 'smtk', 'pyramid.json'],
                'vtk': ['model', '3d', 'exodus', 'disk_out_ref.ex2'],
            }

            for (session_type, path) in session_files.items():
                if session_type == 'native' or session_type in dir(smtk.session):
                    filename = os.path.join(
                        *([smtk.testing.DATA_DIR, ] + path))
                    print('Testing load and close of %s' % filename)
                    if self.loadThenCloseSessionModel(session_type, filename) == False:
                        status = 1

        print('Done')


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
