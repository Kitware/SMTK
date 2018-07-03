//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ResourceSetWriter.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/io/ResourceSetReader.h"

#include "smtk/attribute/Resource.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Set.h"

#include <boost/filesystem.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Test-example for smtk::io::ResourceSetWriter

int main(int argc, const char* argv[])
{
  std::cout << "This test has been disabled until resource links are in place." << std::endl;
  return 125;

  int status = 0;

  // Command line arguments are:
  // * List of input files
  // * Last item is output file

  if (argc < 3)
  {
    std::cout << "\n"
              << "Combines attribute files into single resource file.\n"
              << "Usage: ResourceSetWriterTest input.sbt"
              << "  [input2.sbt ...]  output.crf \n"
              << std::endl;
    return 1;
  }

  smtk::resource::Set resources;

  // Build Set from input filenames
  // This version presumes all attribute resources
  smtk::io::Logger logger;
  std::string link;
  smtk::resource::Set::Role role;

  for (int i = 1; i < argc - 1; ++i)
  {
    const char* input_path = argv[i];

    auto attResource = smtk::attribute::Resource::create();
    smtk::io::AttributeReader reader;
    bool err = reader.read(attResource, input_path, logger);
    if (err)
    {
      std::cerr << "Error loading " << input_path << ": \n" << logger.convertToString() << "\n";
      return -3;
    }

    // Initialize ResourcePtr
    smtk::resource::ResourcePtr resource(attResource);

    // Generate id from index
    // Switch to std::to_string() once c++11 arrives
    std::stringstream ss;
    ss << "att" << i;
    std::string id = ss.str();

    // Make every other input an included link
    link = "";
    role = smtk::resource::Set::TEMPLATE;
    if (i % 2 == 0)
    {
      // Strip off path to just get the filename
      //link  = vtksys::SystemTools::GetFilenameName(input_paths[i]);
      boost::filesystem::path p(input_path);
      link = p.filename().string<std::string>();
      role = smtk::resource::Set::INSTANCE;
    }

    // Add resource to set
    bool ok = resources.add(resource, id, link, role);
    if (!ok)
    {
      std::cerr << "Error adding " << id << " to resource set\n";
      return -4;
    }
  }

  // Write resource set to file
  smtk::io::ResourceSetWriter writer;
  const char* output_path = argv[argc - 1];
  smtk::io::ResourceSetWriter::LinkedFilesOption option =
    smtk::io::ResourceSetWriter::EXPAND_LINKED_FILES;
  bool writeHasErrors = writer.writeFile(output_path, resources, logger, option);
  if (writeHasErrors)
  {
    std::cerr << "ERROR writing " << output_path << "\n";
    status += 1;
  }
  else
  {
    std::cout << "Wrote " << output_path << "\n";
  }

  // Verify readback
  smtk::resource::Set readback_resources;
  smtk::io::ResourceSetReader reader;
  bool hasErrors = reader.readFile(output_path, readback_resources, logger);
  if (hasErrors)
  {
    std::cerr << "Readback had errors\n" << logger.convertToString() << std::endl;
    status += 1;
  }
  std::cout << "Read back " << output_path << std::endl;

  auto numResources = readback_resources.numberOfResources();
  if (numResources != 2)
  {
    std::cerr << "ERROR: Expected to read back 2 resources, got " << numResources << std::endl;
    status += 1;
  }

  return status;
}
