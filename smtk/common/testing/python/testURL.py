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
        urls = ('https://kitware.com/foo/bar',
                'file:///foo/bar',
                'file://baz/foo/bar',
                'http://xyzzy@baz/foo/bar?bleb',
                'http://xyzzy@baz/foo/bar?bleb#flim')
        allParsed = []
        for txt in urls:
            parsed = smtk.common.URL(txt)
            print('"', parsed.scheme().data(),
                  '", "', parsed.authority().data(),
                  '", "', parsed.path().data(),
                  '", "', parsed.query().data(),
                  '", "', parsed.fragment().data(),
                  '"')
            print(parsed.to_string(), txt == parsed.to_string())
            self.assertEqual(parsed.to_string(), txt,
                             'Round-tripped URL was not identical.')
            allParsed.append(parsed)
        self.assertEqual(len(allParsed), len(
            urls), 'Expected to parse every URL.')
        self.assertEqual(allParsed, [smtk.common.URL(x) for x in urls],
                         'Same test in should produce identical URLs out.')
        s1 = set(allParsed)
        s2 = set([smtk.common.URL(x) for x in urls])
        print(s1)
        self.assertEqual(
            s1, s2, 'Hashing should produce identical sets of URLs.')


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
