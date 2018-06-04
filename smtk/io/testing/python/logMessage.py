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
import smtk
import smtk.io
import smtk.testing
import sys


class LogMessage(smtk.testing.TestCase):

    """Test Python log messages.

       This test ensures that the messaging system (SMTK's Python analog to its
       C++ 'smtkInfo/Debug/Warning/ErrorMacro') behaves as expected).
    """

    def testLogMessage(self):
        # construct an instance of the logger
        logger = smtk.io.Logger()

        # test a single message of each type (info, debug, warning, error)
        smtk.InfoMessage(logger, "This is an info message")
        smtk.DebugMessage(logger, "This is a debug message")
        smtk.WarningMessage(logger, "This is a warning message")
        smtk.ErrorMessage(logger, "This is an error message")

        # confirm that all of the messages have been stored
        self.assertEqual(logger.numberOfRecords(), 4)

        # check that the debug, warning and error messages contain the file name
        # that triggered the message (this file!)
        for i in range(logger.numberOfRecords()):
            rec = logger.toString(i, True)
            if 'INFO' not in rec:
                self.assertTrue(__file__ in rec)

if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
