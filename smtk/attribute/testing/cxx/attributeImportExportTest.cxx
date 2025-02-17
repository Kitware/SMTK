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

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/attribute/operators/Export.h"
#include "smtk/attribute/operators/Import.h"

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

    smtk::attribute::Import::Ptr importer = smtk::attribute::Import::create();
    importer->parameters()->findFile("filename")->setValue(infile);
    importer->parameters()->findVoid("UseDirectoryInfo")->setIsEnabled(useDirectoryInfo);

    auto result = importer->operate();
    if (
      result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Import operator failed\n";
      std::cerr << importer->log().convertToString(true) << "\n";
      return -2;
    }

    auto resource = std::dynamic_pointer_cast<smtk::attribute::Resource>(
      result->findResource("resourcesCreated")->value());

    smtk::attribute::Export::Ptr exporter = smtk::attribute::Export::create();
    exporter->parameters()->associate(resource);
    exporter->parameters()->findFile("filename")->setValue(outfile);

    result = exporter->operate();
    if (
      result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Export operator failed\n";
      std::cerr << exporter->log().convertToString(true) << "\n";
      return -2;
    }

    return 0;
  }
}
