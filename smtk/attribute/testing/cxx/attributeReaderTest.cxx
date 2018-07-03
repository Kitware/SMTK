//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Resource.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "nlohmann/json.hpp"
#include "smtk/attribute/json/jsonResource.h"
#include <iostream>

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " attributeFile\n";
    return -1;
  }
  {
    smtk::attribute::ResourcePtr resource = smtk::attribute::Resource::create();
    smtk::io::Logger logger;
    smtk::io::AttributeReader reader;
    if (reader.read(resource, argv[1], logger))
    {
      std::cerr << "Encountered Errors while processing " << argv[1] << "\n";
      std::cerr << logger.convertToString();
      return -2;
    }
    if (logger.numberOfRecords())
    {
      std::cout << logger.convertToString();
    }
    nlohmann::json json = resource;
    std::cout << json.dump(2) << std::endl;
    return 0;
  }
}
