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
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
  std::string arg1, infile, outfile;
  bool useDirectoryInfo = false;
  if (argc == 4)
  {
    arg1 = argv[1];
  }
  if ((argc < 3) || (argc > 4) || ((argc == 4) && (arg1 != "-I")))
  {
    std::cerr << "Usage: " << argv[0] << " -I inputAttributeFile outputAttributeFile\n"
              << "-I (optional) indicates the writer should use include files instead of writing\n"
              << "out a single file\n";
    return -1;
  }
  {
    if (argc == 4)
    {
      useDirectoryInfo = true;
      infile = argv[2];
      outfile = argv[3];
    }
    else
    {
      infile = argv[1];
      outfile = argv[2];
    }
    smtk::attribute::ResourcePtr resource = smtk::attribute::Resource::create();
    smtk::io::Logger logger;
    smtk::io::AttributeReader reader;
    smtk::io::AttributeWriter writer;
    writer.useDirectoryInfo(useDirectoryInfo);
    if (reader.read(resource, infile, true, logger))
    {
      std::cerr << "Encountered Errors while reading " << infile << "\n";
      std::cerr << logger.convertToString();
      return -2;
    }
    if (writer.write(resource, outfile, logger))
    {
      std::cerr << "Encountered Errors while writing " << outfile << "\n";
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
