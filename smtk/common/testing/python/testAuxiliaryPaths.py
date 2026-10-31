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
"""Verify the Python bindings for SMTK auxiliary-path discovery.

Check that search directories are returned as distinct pathlib.Path objects and
include the location relative to the loaded SMTK library. Verify that pathlib
and string arguments are accepted and that missing, absolute, empty, and
parent-traversing requests return None. Filesystem search priority and successful
file lookup are covered by the C++ unitAuxiliaryPaths test.
"""

import pathlib
import unittest
import uuid

import smtk.common
import smtk.testing


class AuxiliaryPathsTest(unittest.TestCase):
    def test_search_paths(self):
        paths = smtk.common.Paths()
        roots = paths.auxiliarySearchPaths()
        self.assertTrue(roots)
        self.assertTrue(all(isinstance(root, pathlib.Path) for root in roots))
        self.assertEqual(len(roots), len(set(roots)))
        library = pathlib.Path(paths.pathToThisLibrary()
                               ).parent / "share" / "smtk"
        self.assertIn(library, roots)

    def test_missing_resource(self):
        paths = smtk.common.Paths()
        missing = pathlib.Path("workflows") / \
            str(uuid.uuid4()) / "missing.smtk"
        self.assertIsNone(paths.findAuxiliaryPath(missing))
        self.assertIsNone(paths.findAuxiliaryPath("../outside.smtk"))
        self.assertIsNone(paths.findAuxiliaryPath(pathlib.Path.cwd()))
        self.assertIsNone(paths.findAuxiliaryPath(""))


if __name__ == "__main__":
    smtk.testing.process_arguments()
    unittest.main()
