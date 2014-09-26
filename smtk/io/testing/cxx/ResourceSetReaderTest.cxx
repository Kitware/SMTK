/*=========================================================================

Copyright (c) 1998-2014 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/


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
      unsigned numResources =  resources.numberOfResources();
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
    } // if (argc > 2)

  return status;
}
