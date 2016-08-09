//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/io/ResourceSetWriter.h"

#include "smtk/attribute/System.h"

#include "smtk/common/ResourceSet.h"

#include <boost/filesystem.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>


// Test-example for smtk::io::ResourceSetWriter

int main(int argc, const char* argv[])
{
  int status = 0;

  // Command line arguments are:
  // * List of input files
  // * Last item is output file

  if (argc < 3)
    {
    std::cout << "\n"
              << "Combines attribute files into single resource file.\n"
              << "Usage: ResourceSetWriterTest input.sbt"
              <<"  [input2.sbt ...]  output.crf \n"
              << std::endl;
    return 1;
    }


  smtk::common::ResourceSet resources;

  // Build ResourceSet from input filenames
  // This version presumes all attribute systems
  smtk::io::Logger logger;
  std::string link;
  smtk::common::ResourceSet::ResourceRole role;

  for (int i=1; i<argc-1; ++i)
    {
    const char *input_path = argv[i];

    smtk::attribute::System *system = new smtk::attribute::System();
    smtk::io::AttributeReader reader;
    bool err = reader.read(*system, input_path, logger);
    if (err)
      {
      std::cerr << "Error loading " << input_path << ": \n"
                << logger.convertToString() << "\n";
      return -3;
      }

    // Initialize ResourcePtr
    smtk::common::ResourcePtr resource(system);

    // Generate id from index
    // Switch to std::to_string() once c++11 arrives
    std::stringstream ss;
    ss << "att" << i;
    std::string id = ss.str();

    // Make every other input an included link
    link = "";
    role = smtk::common::ResourceSet::TEMPLATE;
    if (i % 2 == 0)
      {
      // Strip off path to just get the filename
      //link  = vtksys::SystemTools::GetFilenameName(input_paths[i]);
      boost::filesystem::path p(input_path);
      link = p.filename().string<std::string>();
      role = smtk::common::ResourceSet::INSTANCE;
      }

    // Add resource to set
    bool ok = resources.addResource(resource, id, link, role);
    if (!ok)
      {
      std::cerr << "Error adding " << id << " to resource set\n";
      return -4;
      }
    }

  // Write resource set to file
  smtk::io::ResourceSetWriter writer;
  const char *output_path = argv[argc-1];
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

  return status;
}
