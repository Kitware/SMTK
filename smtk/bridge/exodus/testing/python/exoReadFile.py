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

try:
  import unittest2 as unittest
except ImportError:
  import unittest

class TestExodusSession(unittest.TestCase):

  def setUp(self):
    import os, sys
    self.filename = os.path.join(smtk.testing.DATA_DIR, 'exodus', 'disk_out_ref.ex2')

    self.mgr = smtk.model.Manager.create()
    sess = self.mgr.createSession('exodus')
    SetActiveSession(sess)

  def testRead(self):
    ents = Read(self.filename)
    self.model = smtk.model.Model(ents[0])

    #Verify that the file contains the proper number of groups.
    numGroups = len(self.model.groups())
    self.assertEqual(numGroups, 11, 'Expected 11 groups, found %d' % numGroups)

    #Verify that the group names match those from the Exodus file.
    nameset = set([
      'Unnamed block ID: 1 Type: HEX8',
      'Unnamed set ID: 1',
      'Unnamed set ID: 2',
      'Unnamed set ID: 3',
      'Unnamed set ID: 4',
      'Unnamed set ID: 5',
      'Unnamed set ID: 6',
      'Unnamed set ID: 7'])
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

if __name__ == '__main__':
  smtk.testing.process_arguments()
  unittest.main()
