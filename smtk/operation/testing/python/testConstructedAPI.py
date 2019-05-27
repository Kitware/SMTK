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
import unittest
import smtk
import smtk.attribute
import smtk.operation
import smtk.testing
import re


class TestConstructedAPI(smtk.testing.TestCase):

    def testConstructedAPI(self):
        op = smtk.operation.ReadResource.create()
        parameters = op.parameters()
        print(dir(parameters))
        assert(hasattr(parameters, 'setFilename'))
        assert(hasattr(parameters, 'filename'))
        assert(hasattr(parameters, 'setDebugLevel'))
        assert(hasattr(parameters, 'debugLevel'))
        assert(hasattr(parameters, 'enableDebugLevel'))
        assert(hasattr(parameters, 'debugLevelEnabled'))


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
