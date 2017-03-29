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
import smtk.testing
from smtk.simple import *


class CGMTransforms(smtk.testing.TestCase):

    def setUp(self):
        self.mgr = smtk.model.Manager.create()
        sref = self.mgr.createSession('cgm', smtk.model.SessionRef())
        sref.assignDefaultName()
        SetActiveSession(sref)

    def testTransforms(self):
        brick1 = CreateBrick(width=0.5)
        brick2 = CreateBrick(width=0.5)

        brick3 = Translate(brick2, [0.5, 0.0, 0.0])[0]
        if not brick3 or brick3.entity() != brick2.entity():
            print "Expecting entities to match: %s != %s" % (brick2.entity(), brick3.entity())
            sys.exit(1)

        brick4 = Rotate(brick3, angle=60.0, center=[
                        0.5, 0.0, 0.0], axis=[0.3333, 0.6667, 0.6667])[0]
        if not brick4 or brick4.entity() != brick3.entity():
            print "Expecting entities to match: %s != %s" % (brick3.entity(), brick4.entity())
            sys.exit(1)

        brick5 = Union([brick1, brick4])
        print brick5, brick5.name(), brick5.flagSummary(0)
        #brick6 = Scale([brick5], [3.0, 2.0, 1.0])[0]

        print self.haveVTK(), self.haveVTKExtension()
        if self.haveVTK() and self.haveVTKExtension():

            colormap = {
                # brick3: '#5a5255',
                # brick4: '#ae5a41',
                brick5: '#559e83'
                # brick6: '#c3cb71'
                #'#1b85b8',
                #'#cb2c31',
                #'#8b1ec4',
                #'#ff6700'
            }

            self.startRenderTest()

            # Render groups with colors:
            for body, color in colormap.iteritems():
                color = self.hex2rgb(color)
                SetEntityProperty(body, 'color', as_float=color)
                self.addModelToScene(body)

            self.renderer.SetBackground(1, 1, 1)
            cam = self.renderer.GetActiveCamera()
            cam.SetFocalPoint(0., 0., 0.)
            cam.SetPosition(19, 17, -43)
            cam.SetViewUp(-0.891963, -0.122107, -0.435306)
            self.renderer.ResetCamera()
            self.renderWindow.Render()
            self.assertImageMatchIfFileExists(
                ['baseline', 'smtk', 'cgm', 'transforms.png'])
            self.interact()

        else:
            self.assertFalse(
                self.haveVTKExtension(),
                'Could not import vtk. Python path is {pp}'.format(pp=sys.path))

        # Now verify that self.mgr.closeSession removes the entity record for
        # the session.
        sref = GetActiveSession()
        SetActiveSession(smtk.model.SessionRef())
        # self.mgr.closeSession(sref)
        #sys.exit(0 if sref.name() == ('invalid id ' + str(sref.entity())) else 1)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
