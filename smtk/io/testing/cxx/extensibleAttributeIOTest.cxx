//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <iostream>
#include <sstream>

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/System.h"

#include "smtk/common/UUID.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{
std::string write_root = SMTK_SCRATCH_DIR;

void cleanup(const std::string& file_path)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}

const char* testInput =
  "<?xml version=\"1.0\" encoding=\"utf-8\" ?>                                   "
  "<SMTK_AttributeSystem Version=\"2\">                                          "
  "  <Definitions>                                                               "
  "    <AttDef Type=\"att1\" BaseType=\"\">                                      "
  "      <ItemDefinitions>                                                       "
  "	<String Name=\"myStrings\" Extensible=\"1\"                            "
  "               NumberOfRequiredValues=\"1\">                                  "
  "	</String>                                                              "
  "	<File Name=\"myFiles\" Extensible=\"1\" ShouldExist=\"false\"          "
  "               NumberOfRequiredValues=\"1\">                                  "
  "	</File>                                                                "
  "      </ItemDefinitions>                                                      "
  "    </AttDef>                                                                 "
  "  </Definitions>                                                              "
  "  <Attributes>                                                                "
  "    <Att Name=\"att\" Type=\"att1\"/>                                         "
  "  </Attributes>                                                               "
  "</SMTK_AttributeSystem>                                                       ";
}

int main()
{
  smtk::attribute::System system;
  smtk::io::Logger logger;
  smtk::io::AttributeReader reader;

  if (reader.readContents(system, testInput, logger))
  {
    std::cerr << "Encountered Errors while reading input data\n";
    std::cerr << logger.convertToString();
    return -2;
  }

  std::vector<smtk::attribute::AttributePtr> atts;
  system.attributes(atts);
  if (atts.size() != 1)
  {
    std::cerr << "Unexpected number of attributes: " << atts.size() << "\n";
    std::cerr << logger.convertToString();
    return -2;
  }
  smtk::attribute::AttributePtr att = atts[0];

  smtk::attribute::StringItemPtr myStrings = att->findString("myStrings");
  myStrings->setNumberOfValues(2);
  myStrings->setValue(0, "string0");
  myStrings->setValue(1, "string1");

  smtk::attribute::FileItemPtr myFiles = att->findFile("myFiles");
  myFiles->setNumberOfValues(2);
  myFiles->setValue(0, "/path/to/file0");
  myFiles->setValue(1, "/path/to/file1");

  if (!att->isValid())
  {
    std::cerr << "Input attributes are invalid\n";
    std::cerr << logger.convertToString();
    return -2;
  }

  std::stringstream s;
  s << write_root << "/" << smtk::common::UUID::random().toString() << ".xml";
  std::string fileName = s.str();

  smtk::io::AttributeWriter writer;

  if (writer.write(system, fileName, logger))
  {
    std::cerr << "Failed to write to " << fileName << "\n";
    std::cerr << logger.convertToString();
    return -2;
  }

  smtk::attribute::System copiedSystem;
  if (reader.read(copiedSystem, fileName, logger))
  {
    std::cerr << "Failed to read from " << fileName << "\n";
    std::cerr << logger.convertToString();
    return -2;
  }

  {
    std::vector<smtk::attribute::AttributePtr> copiedAtts;
    copiedSystem.attributes(copiedAtts);
    if (copiedAtts.size() != 1)
    {
      std::cerr << "Unexpected number of attributes: " << copiedAtts.size() << "\n";
      std::cerr << logger.convertToString();
      return -2;
    }

    smtk::attribute::AttributePtr copiedAtt = copiedAtts[0];
    if (!copiedAtt->isValid())
    {
      std::cerr << "Copied attributes are invalid\n";
      std::cerr << logger.convertToString();
      return -2;
    }

    smtk::attribute::StringItemPtr myCopiedStrings = copiedAtt->findString("myStrings");
    smtk::attribute::ItemPtr myStringsAsItems =
      std::static_pointer_cast<smtk::attribute::Item>(myStrings);
    smtk::attribute::ConstItemPtr myStringsAsConstItems =
      std::const_pointer_cast<smtk::attribute::Item>(myStringsAsItems);
    myCopiedStrings->assign(myStringsAsConstItems);

    if (myCopiedStrings->numberOfValues() != myStrings->numberOfValues())
    {
      std::cerr << "Unexpected number of string values: " << myCopiedStrings->numberOfValues()
                << " vs " << myStrings->numberOfValues() << "\n";
      std::cerr << logger.convertToString();
      return -2;
    }

    smtk::attribute::FileItemPtr myCopiedFiles = copiedAtt->findFile("myFiles");
    smtk::attribute::ItemPtr myFilesAsItems =
      std::static_pointer_cast<smtk::attribute::Item>(myFiles);
    smtk::attribute::ConstItemPtr myFilesAsConstItems =
      std::const_pointer_cast<smtk::attribute::Item>(myFilesAsItems);
    myCopiedFiles->assign(myFilesAsConstItems);

    if (myCopiedFiles->numberOfValues() != myFiles->numberOfValues())
    {
      std::cerr << "Unexpected number of file values: " << myCopiedFiles->numberOfValues() << " vs "
                << myFiles->numberOfValues() << "\n";
      std::cerr << logger.convertToString();
      return -2;
    }
  }

  cleanup(fileName);

  return 0;
}
