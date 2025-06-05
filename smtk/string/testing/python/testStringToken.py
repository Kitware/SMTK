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


class TestStringToken(unittest.TestCase):

    def test(self):
        blank = smtk.string.Token()
        print(blank)
        self.assertFalse(
            blank.valid(), 'Default-constructed Token should be invalid.')
        texts = ('a', 'b', 'c', 'aa')
        allParsed = []
        for txt in texts:
            parsed = smtk.string.Token(txt)
            print(parsed)
            self.assertEqual(str(parsed), txt,
                             'Round-tripped Token was not identical.')
            allParsed.append(parsed)
        self.assertEqual(len(allParsed), len(texts),
                         'Expected to parse every Token.')
        self.assertEqual(allParsed, [smtk.string.Token(x) for x in texts],
                         'Same text in should produce identical Tokens out.')
        s1 = set(allParsed)
        s2 = set([smtk.string.Token(x) for x in texts])
        print(s1)
        self.assertEqual(
            s1, s2, 'Hashing should produce identical sets of Tokens.')


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
