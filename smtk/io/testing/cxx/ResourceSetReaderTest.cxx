//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/io/ResourceSetReader.h"

#include "smtk/common/ResourceSet.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>


// Test-example for CMBResourceSetWriter

int main(int argc, const char* argv[])
{
  if (argc < 2)
    {
    std::cout << "\n"
              << "Reads resource with 1 or more attribute managers.\n"
              << "Usage: ResourceSetReaderTest resource_file"
              << "  [expect_number_of_resources]"
              << std::endl;
    return 1;
    }

  int status = 0;  // return value

  // Instantiate empty resource set and reader
  smtk::common::ResourceSet resources;
  smtk::io::ResourceSetReader reader;
  smtk::io::Logger logger;

  std::string input_path = argv[1];
  bool hasErrors = reader.readFile(input_path, resources, logger);
  if (hasErrors)
    {
    std::cerr << "Reader had errors\n"
              << logger.convertToString()
              << std::endl;
    status += 1;
    }

  // Test number of resources if specified
  if (argc > 2)
    {
    unsigned expectedNumber = 0;
    std::stringstream convert(argv[2]);
    if (!(convert >> expectedNumber))
      {
      std::cerr << "ERROR: argv[2] not an unsigned integer: "
                << argv[2] << std::endl;
      status += 1;
      }
    else
      {
      unsigned numResources =
        static_cast<unsigned>(resources.numberOfResources());
      if (numResources != expectedNumber)
        {
        std::cerr << "ERROR: Expecting " << expectedNumber
                  << " resources, loaded "
                  << numResources << std::endl;
        status += 1;
        }
      else
        {
        std::cout << "Number of resources loaded: "
                  << numResources << "\n";
        }

      // Dump out resource ids for info only
      std::vector<std::string> resourceIds = resources.resourceIds();
      for (unsigned i=0; i < numResources; ++i)
        {
        std::cout << "  " << resourceIds[i] << "\n";
        }
      }  // else

    smtk::common::ResourcePtr resource;
    resources.get("att0", resource);
    smtk::common::Resource::Type resType = resource->resourceType();
    std::cout << "att0 type: " << smtk::common::Resource::type2String(resType) << std::endl;
    } // if (argc > 2)

  return status;
}
