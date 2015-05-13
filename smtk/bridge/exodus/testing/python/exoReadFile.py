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
import smtk.testing
from smtk.simple import *

def hex2rgb(hexstr):
  hh = hexstr[1:] if hexstr[0] == '#' else hexstr
  rr = int(hh[0:2],16) / 255.
  gg = int(hh[2:4],16) / 255.
  bb = int(hh[4:6],16) / 255.
  return (rr, gg, bb)

class TestExodusSession(smtk.testing.TestCase):

  def setUp(self):
    import os, sys
    self.filename = os.path.join(smtk.testing.DATA_DIR, 'exodus', 'disk_out_ref.ex2')

    self.mgr = smtk.model.Manager.create()
    sess = self.mgr.createSession('exodus')
    SetActiveSession(sess)

  def testRead(self):
    ents = Read(self.filename)
    self.model = smtk.model.Model(ents[0])

    #Verify that the model is marked as discrete
    self.assertEqual(
        self.model.geometryStyle(), smtk.model.DISCRETE,
        'Expected discrete model, got {gs}'.format(gs=self.model.geometryStyle()))

    #Verify that the file contains the proper number of groups.
    numGroups = len(self.model.groups())
    self.assertEqual(numGroups, 11, 'Expected 11 groups, found %d' % numGroups)

    #Verify that the group names match those from the Exodus file.
    nameset = {
        'Unnamed block ID: 1 Type: HEX8': '#5a5255',
        'Unnamed set ID: 1':              '#ae5a41',
        'Unnamed set ID: 2':              '#559e83',
        'Unnamed set ID: 3':              '#c3cb71',
        'Unnamed set ID: 4':              '#1b85b8',
        'Unnamed set ID: 5':              '#cb2c31',
        'Unnamed set ID: 6':              '#8b1ec4',
        'Unnamed set ID: 7':              '#ff6700'
    }
    self.assertTrue(all([x.name() in nameset for x in self.model.groups()]),
        'Not all group names recognized.')

    # Verify that no groups which are not in the list above are present.
    groupnames = [x.name() for x in self.model.groups()]
    self.assertTrue(all([x in groupnames for x in nameset]),
        'Some expected group names not present.')

    # Count the number of each *type* of group (node, face, volume)
    grouptypes = [x.flagSummary() for x in self.model.groups()]
    gtc = {x:grouptypes.count(x) for x in grouptypes}
    expectedgrouptypecounts = {
      'boundary group (0-d entities)': 3,
      'boundary group (0,1,2-d entities)': 7,
      'domain group (3-d entities)': 1
      }
    for entry in gtc.items():
      print '%40s: %d' % entry
    self.assertEqual(gtc, expectedgrouptypecounts,
        'At least one group was of the wrong type.')

    if self.haveVTK() and self.haveVTKExtension():

      # Render groups with colors:
      for grp in self.model.groups():
        color = hex2rgb(nameset[grp.name()])
        SetEntityProperty(grp, 'color', as_float=color)

      self.startRenderTest()
      mbs = self.addModelToScene(self.model)

      self.renderer.SetBackground(1,1,1)
      cam = self.renderer.GetActiveCamera()
      cam.SetFocalPoint(0., 0., 0.)
      cam.SetPosition(19,17,-43)
      cam.SetViewUp(-0.891963, -0.122107, -0.435306)
      self.renderer.ResetCamera()
      self.renderWindow.Render()
      self.assertImageMatch(['baselines', 'exodus', 'disk_out_ref.png'])
      self.interact()

if __name__ == '__main__':
  smtk.testing.process_arguments()
  smtk.testing.main()
