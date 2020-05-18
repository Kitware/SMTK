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
Try running a "universal" operator on an imported model.
"""
from __future__ import print_function
import os
import sys
import smtk
import smtk.io
import smtk.model
import smtk.testing
from uuid import uuid4
import unittest


class TestModelRegressions(unittest.TestCase):

    def testGroupMismatch(self):
        """Groups should not match cells when finding entities by type."""

        rsrc = smtk.model.Resource.create()
        rsrc.setName('foo')
        mask = smtk.model.VOLUME
        g = rsrc.addGroup(mask, 'Foo')
        v1 = rsrc.addVolume()
        v2 = rsrc.addVolume()
        zz = rsrc.findEntitiesOfType(mask, True)
        yy = [vol for vol in rsrc.findEntitiesOfType(
            mask, True) if vol.isVolume()]
        xx = rsrc.findEntitiesOfType(mask, False)
        print('unfiltered set: %d entries, filtered set: %d entries' %
              (len(zz), len(yy)))
        self.assertEqual(
            len(zz), 2, 'findEntitiesOfType failed with exactMatch on')
        self.assertEqual(
            len(xx), 3, 'findEntitiesOfType failed with exactMatch off')
        self.assertEqual(len(yy), 2, 'filtering on volume.isValid failed')
        print('Done')


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
