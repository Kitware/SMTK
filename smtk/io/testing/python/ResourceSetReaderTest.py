#!/usr/bin/python
import os
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


def TestResourceSetReader(readFromFile):
    """A test for ResourceSetReader
    It is a direct port of the Cxx version"""

    if len(sys.argv) < 2:
        print "Reads resource with 1 or more attribute managers"
        print "Usage: ResourceSetReaderTest resource_file"
        print "  [expect_number_of_resources]"
        return 1

    status = 0  # return value

    resources = smtk.common.ResourceSet()
    print 'resources', resources
    reader = smtk.io.ResourceSetReader()
    logger = smtk.io.Logger()

    input_path = sys.argv[1]

    if readFromFile:
        hasErrors = reader.readFile(input_path, resources, logger)
    else:
        with open(input_path, 'r') as myfile:
            data = myfile.read().replace('\n', '')

        resources.setLinkStartPath(os.path.dirname(input_path))
        hasErrors = reader.readString(data, resources, logger, True)

    if hasErrors:
        print "Reader has errors"
        print logger.convertToString()
        status = status + 1

    expectedNumber = 0
    convert = sys.argv[2]

    try:
        expectedNumber = int(convert)
    except:
        status = status + 1
    finally:
        if expectedNumber < 0:
            print "ERROR: argv[2] not an unsigned integer"
            status = status + 1
        else:
            numResources = resources.numberOfResources()
            if numResources != expectedNumber:
                print "ERROR: Expecting ", expectedNumber, \
                    " resources, loaded ", numResources
                status = status + 1
            else:
                print "Number of resources loaded:", numResources

            # dump out resource ids for info only
            resourceIds = resources.resourceIds()
            for id in resourceIds:
                print id

    #print ('dir'), dir(resources)
    # print help(resources.get)
    res0 = resources.get('att0')
    print 'type', res0.resourceType()
    # print dir(res0)

    return status


if __name__ == '__main__':
    sys.exit(TestResourceSetReader(True) + TestResourceSetReader(False))
