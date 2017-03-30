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
import unittest
import smtk
from smtk import common
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
        intversion = (1000 * (100 * major + minor) + patch)

        self.assertEqual(
            number, mmp, 'Version number should be formatted as major.minor.patch.')
        self.assertEqual(combined, intversion,
                         'Combined integer version miscalculated.')

        readme = open(os.path.join(smtk.testing.SOURCE_DIR, 'doc/index.rst'))
        readme.readline()  # Skip title flare
        readme.readline()  # Skip title
        readmeVersion = readme.readline().strip()
        readmeMatch = 'Version %s' % number

        self.assertEqual(readmeVersion, readmeMatch,
                         """Mismatched version numbers in CMakeLists.txt and ReadMe.mkd.

           Do not take version bumps lightheartedly.
           Do not remove this test nor the line in the ReadMe.mkd
           with the explicit version number; it is your friend.

           This test exists as a reminder to follow the release
           procedure and verify that git branches and tags, signed
           source and/or binary packages, updated documentation,
           and all other release artifacts have been produced.
           All tests should be passing and all issues assigned to
           the milestone for this release (you **do** have a
           milestone for this release, right?) have been closed.
        """)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
