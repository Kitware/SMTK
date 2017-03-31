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
import smtk
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.bridge.cgm
    import smtk.model
import smtk.testing


class TestCGMSolidModeling(smtk.testing.TestCase):

    def setUp(self):
        self.mgr = smtk.model.Manager.create()
        self.sref = self.mgr.createSession('cgm', smtk.model.SessionRef())
        sess = self.sref.session()
        self.sref.assignDefaultName()
        print '\n\n%s: type "%s" %s %s' % \
            (self.sref.name(), sess.name(),
             self.sref.flagSummary(0), sess.sessionId())
        print '  Site: %s' % (self.sref.site() or 'local')
        for eng in self.sref.engines():
            print '  Engine %s filetypes:\n    %s' % \
                (eng, '\n    '.join(self.sref.fileTypes(eng)))
        # We could evaluate the session tag as JSON, but most of
        # the information is available through methods above that
        # we needed to test:
        sessiontag = self.sref.tag()
        print '\n'

        opnames = self.sref.operatorNames()

    def testSolidModelingOps(self):
        cs1 = self.sref.op('create sphere')
        cs1.findAsDouble('radius').setValue(1.)
        # cs1.findAsDouble('inner radius').setValue(0.1) # Crashes
        # cs1.findAsDouble('inner radius').setValue(-0.1) # Complains bitterly
        cs1.findAsDouble('inner radius').setValue(0.2)  # Actually works

        # CGM's OCC backend apparently does not pay attention to
        # the sphere center parameters:
        cs1.findAsDouble('center').setValue(0, 0.2)
        cs1.findAsDouble('center').setValue(1, 0.2)
        cs1.findAsDouble('center').setValue(2, 0.2)

        res = cs1.operate()
        sph = res.findModelEntity('created').value(0)

        cs2 = self.sref.op('create sphere')
        cs2.findAsDouble('radius').setValue(0.5)
        cs2.findAsDouble('center').setValue(0, 0.9)
        res2 = cs2.operate()
        sph2 = res2.findModelEntity('created').value(0)

        print 'Operators that can associate with ' + sph2.flagSummary(1) + ' include\n  %s' % \
            '\n  '.join(self.sref.operatorsForAssociation(sph2.entityFlags()))

        u1 = self.sref.op('union')
        u1.associateEntity(sph)
        u1.associateEntity(sph2)
        res = u1.operate()
        # You will see:
        #    Updated volume(s): 2
        #    Destroyed volume(s): 1
        su = res.findModelEntity('modified').value(0)
        # Note that su has same UUID as sph2

        # Test cylinder creation.
        from smtk.simple import *
        SetActiveSession(self.sref)
        cyl = CreateCylinder(top_radius=1.0)

        #json = smtk.io.SaveJSON.fromModelManager(self.mgr)
        #cylFile = open('cyl.json', 'w')
        #print >> cylFile, json
        # cylFile.close()

        # Now verify that self.mgr.closeSession removes the entity record for
        # the session.
        self.mgr.closeSession(self.sref)
        self.assertEqual(
            self.sref.name(),
            'invalid id {uid}'.format(uid=str(self.sref.entity())),
            'Expected invalid session name after closing, got "{s}"'.format(
                s=self.sref.name()))


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
