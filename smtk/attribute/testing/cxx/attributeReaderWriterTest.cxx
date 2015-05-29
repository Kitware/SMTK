//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/System.h"

#include "smtk/io/Logger.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"

#include <iostream>

int main(int argc, char *argv[])
{
  if (argc != 3)
    {
    std::cerr << "Usage: " << argv[0] << " inputAttributeFile outputAttributeFile\n";
    return -1;
    }
  {
  smtk::attribute::System system;
  smtk::io::Logger logger;
  smtk::io::AttributeReader reader;
  smtk::io::AttributeWriter writer;
  if (reader.read(system, argv[1], logger))
    {
    std::cerr << "Encountered Errors while reading " << argv[1] << "\n";
    std::cerr << logger.convertToString();
    return -2;
    }
  if (writer.write(system, argv[2], logger))
    {
    std::cerr << "Encountered Errors while writing " << argv[2] << "\n";
    std::cerr << logger.convertToString();
    return -2;
    }
  if (logger.numberOfRecords())
    {
    std::cout << logger.convertToString();
    }
  return 0;
  }
}
