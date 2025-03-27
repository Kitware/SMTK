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
import os
import sys
import unittest
import smtk
import smtk.string
import smtk.common
from smtk import common
import smtk.testing
import uuid


class TestURL(unittest.TestCase):

    def test(self):
        blank = smtk.common.URL()
        self.assertFalse(
            blank.valid(), 'Default-constructed URL should be invalid.')
        for txt in ('https://kitware.com/foo/bar', 'file:///foo/bar', 'file://baz/foo/bar', 'http://xyzzy@baz/foo/bar?bleb#flim'):
            parsed = smtk.common.URL(txt)
            print('"', parsed.scheme().data(), '", "', parsed.authority().data(), '", "', parsed.path(
            ).data(), '", "', parsed.query().data(), '", "', parsed.fragment().data(), '"')
            print(parsed.to_string(), txt == parsed.to_string())
            self.assertEqual(parsed.to_string(), txt,
                             'Round-tripped URL was not identical')


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
