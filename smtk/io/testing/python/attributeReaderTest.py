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
    import smtk.attribute
    import smtk.io
from smtk.simple import *


class TestAttributeReader():
    """A test for AttributeReader"""

    def setUp(self):
        if len(sys.argv) < 2:
            print "Reads attribute system file"
            print "Usage: AttributeReaderTest attribute_file"
            print "  [expect_number_of_definitions]"
            print "  [expect_number_of_attributes]"
            return 1

        self.status = 0  # return value

        self.attsys = smtk.attribute.System()
        self.reader = smtk.io.AttributeReader()
        self.logger = smtk.io.Logger()

        input_path = sys.argv[1]

        hasErrors = self.reader.read(self.attsys, input_path, self.logger)
        if hasErrors:
            print "Reader has errors"
            print self.logger.convertToString()
            self.status = self.status + 1

    def testSimpleRead(self):
        # Test definition count
        if len(sys.argv) <= 2:
            return 1

        expectedDefinitionCount = 0
        convert = sys.argv[2]

        try:
            expectedDefinitionCount = int(convert)
        except:
            self.status = self.status + 1
        finally:
            if expectedDefinitionCount < 0:
                print "ERROR: argv[2] not an unsigned integer"
                self.status = self.status + 1
            else:
                definitionList = self.attsys.definitions()
                if len(definitionList) != expectedDefinitionCount:
                    print "ERROR: Expecting ", expectedDefinitionCount, \
                        " definitions, loaded ", len(definitionList)
                    self.status = self.status + 1
                else:
                    print "Number of definitions loaded:", len(definitionList)
                    # for i,defn in enumerate(definitionList):
                    #    print i, defn.type(), defn.isAbstract()

        # Test attribute count
        if len(sys.argv) <= 3:
            return self.status

        expectedAttributeCount = 0
        convert = sys.argv[3]

        try:
            expectedAttributeCount = int(convert)
        except:
            self.status = self.status + 1
        finally:
            if expectedAttributeCount < 0:
                print "ERROR: argv[2] not an unsigned integer"
                self.status = self.status + 1
            else:
                attributeList = self.attsys.attributes()
                if len(attributeList) != expectedAttributeCount:
                    print "ERROR: Expecting ", expectedAttributeCount, \
                        " attributes, loaded ", len(attributeList)
                    self.status = self.status + 1
                else:
                    print "Number of attributes loaded:", len(attributeList)
                    # for i,att in enumerate(attributeList):
                    #    print i, att.name()

        return self.status


if __name__ == '__main__':
    t = TestAttributeReader()
    t.setUp()
    sys.exit(t.testSimpleRead())
