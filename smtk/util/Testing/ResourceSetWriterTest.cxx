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


#include "smtk/attribute/Manager.h"
#include "smtk/util/AttributeReader.h"
#include "smtk/util/Logger.h"
#include "smtk/util/ResourceSet.h"
#include "smtk/util/ResourceSetWriter.h"

#include <boost/filesystem.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>


// Test-example for smtk::util::ResourceSetWriter

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


  smtk::util::ResourceSet resources;

  // Build ResourceSet from input filenames
  // This version presumes all attribute managers
  smtk::util::Logger logger;
  std::string link;
  smtk::util::ResourceSet::ResourceRole role;

  for (unsigned i=1; i<argc-1; ++i)
    {
    const char *input_path = argv[i];

    smtk::attribute::Manager *manager = new smtk::attribute::Manager();
    smtk::util::AttributeReader reader;
    bool err = reader.read(*manager, input_path, logger);
    if (err)
      {
      std::cerr << "Error loading " << input_path << ": \n"
                << logger.convertToString() << "\n";
      return -3;
      }

    // Initialize ResourcePtr
    smtk::util::ResourcePtr resource(manager);

    // Generate id from index
    // Switch to std::to_string() once c++11 arrives
    std::stringstream ss;
    ss << "att" << i;
    std::string id = ss.str();

    // Make every other input an included link
    link = "";
    role = smtk::util::ResourceSet::TEMPLATE;
    if (i % 2 == 0)
      {
      // Strip off path to just get the filename
      //link  = vtksys::SystemTools::GetFilenameName(input_paths[i]);
      boost::filesystem::path p(input_path);
      link = p.filename().c_str();
      role = smtk::util::ResourceSet::INSTANCE;
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
  smtk::util::ResourceSetWriter writer;
  const char *output_path = argv[argc-1];
  bool writeHasErrors = writer.writeFile(output_path, resources, logger);
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
