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
import unittest
import smtk
import smtk.attribute
import smtk.operation
import smtk.testing
import re


class TestConstructedAPI(smtk.testing.TestCase):

    def testConstructedAPI(self):
        op = smtk.operation.WriteResource.create()
        parameters = op.parameters()
        assert (hasattr(parameters, 'setFilename'))
        assert (hasattr(parameters, 'filename'))
        filename = 'foo.smtk'
        parameters.setFilename(filename)
        assert (parameters.filename() == filename)
        assert (hasattr(parameters, 'setDebugLevel'))
        assert (hasattr(parameters, 'debugLevel'))
        assert (hasattr(parameters, 'enableDebugLevel'))
        assert (hasattr(parameters, 'debugLevelEnabled'))
        parameters.enableDebugLevel(True)
        assert (parameters.debugLevelEnabled() == True)
        parameters.setDebugLevel(3)
        assert (parameters.debugLevel() == 3)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
