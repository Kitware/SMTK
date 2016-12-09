#!/usr/bin/python
import sys
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
import smtk
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.common
    import smtk.io
from smtk.simple import *

class TestResourceSetReader():
    """A test for ResourceSetReader
    It is a direct port of the Cxx version"""

    def setUp(self):
        if len(sys.argv) < 2:
            print "Reads resource with 1 or more attribute managers"
            print "Usage: ResourceSetReaderTest resource_file"
            print "  [expect_number_of_resources]"
            return 1

        self.status = 0  # return value

        self.resources = smtk.common.ResourceSet()
        print 'resources', self.resources
        self.reader = smtk.io.ResourceSetReader()
        self.logger = smtk.io.Logger()

        input_path = sys.argv[1]

        hasErrors = self.reader.readFile(input_path, self.resources, self.logger)
        if hasErrors:
            print "Reader has errors"
            print self.logger.convertToString()
            self.status = self.status + 1


    def testSimpleRead(self):
        if len(sys.argv) <= 2:
            return 1

        expectedNumber = 0
        convert = sys.argv[2]

        try:
            expectedNumber = int(convert)
        except:
            self.status = self.status + 1
        finally:
            if expectedNumber < 0:
                print "ERROR: argv[2] not an unsigned integer"
                self.status = self.status + 1
            else:
                numResources = self.resources.numberOfResources()
                if numResources != expectedNumber:
                    print "ERROR: Expecting ", expectedNumber, \
                    " resources, loaded ", numResources
                    self.status = self.status + 1
                else:
                    print "Number of resources loaded:", numResources

                # dump out resource ids for info only
                resourceIds = self.resources.resourceIds()
                for id in resourceIds:
                    print id

        #print ('dir'), dir(self.resources)
        #print help(self.resources.get)
        res0 = self.resources.get('att0')
        print 'type', res0.resourceType()
        #print dir(res0)

        return self.status


if __name__ == '__main__':
    t = TestResourceSetReader()
    t.setUp()
    sys.exit(t.testSimpleRead())
