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
import os, sys
import unittest
import smtk
import smtk.testing
from smtk.simple import *

class Version(unittest.TestCase):

  def test(self):
    major = smtk.common.Version.major()
    minor = smtk.common.Version.minor()
    patch = smtk.common.Version.patch()
    combined = smtk.common.Version.combined()
    number = smtk.common.Version.number()

    mmp = '%d.%d.%d' % (major, minor, patch)
    intversion = (1000 * ( 100 * major + minor ) + patch)
    self.assertEqual(number, mmp, 'Version number should be formatted as major.minor.patch.')
    self.assertEqual(combined, intversion, 'Combined integer version miscalculated.')

if __name__ == '__main__':
  smtk.testing.process_arguments()
  unittest.main()
