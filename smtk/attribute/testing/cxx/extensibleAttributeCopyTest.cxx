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
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/StringItem.h"
#include <iostream>

namespace
{
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
"</SMTK_AttributeSystem>                                                       "
;
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
    std::cerr << "Unexpected number of attributes: "<<atts.size()<<"\n";
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

  {
  std::vector<smtk::attribute::AttributePtr> copiedAtts;
  system.attributes(copiedAtts);
  if (copiedAtts.size() != 1)
    {
    std::cerr << "Unexpected number of attributes: "<<copiedAtts.size()<<"\n";
    std::cerr << logger.convertToString();
    return -2;
    }

  smtk::attribute::AttributePtr copiedAtt = copiedAtts[0];

  smtk::attribute::StringItemPtr myCopiedStrings =
    copiedAtt->findString("myStrings");
  smtk::attribute::ItemPtr myStringsAsItems =
    std::static_pointer_cast<smtk::attribute::Item>(myStrings);
  smtk::attribute::ConstItemPtr myStringsAsConstItems =
    std::const_pointer_cast<smtk::attribute::Item>(myStringsAsItems);
  myCopiedStrings->assign(myStringsAsConstItems);

  if (myCopiedStrings->numberOfValues() != myStrings->numberOfValues())
    {
    std::cerr << "Unexpected number of string values: "<<myCopiedStrings->numberOfValues()<<" vs "<<myStrings->numberOfValues()<<"\n";
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
    std::cerr << "Unexpected number of file values: "<<myCopiedFiles->numberOfValues()<<" vs "<<myFiles->numberOfValues()<<"\n";
    std::cerr << logger.convertToString();
    return -2;
    }
  }

  {
  smtk::attribute::AttributePtr copiedAtt = system.copyAttribute(att);

  if (!copiedAtt)
    {
    std::cerr << "Failed to copy attribute" << "\n";
    std::cerr << logger.convertToString();
    return -2;
    }

  if (!copiedAtt->isValid())
    {
    std::cerr << "Copied attribute is invalid" << "\n";
    std::cerr << logger.convertToString();
    return -2;
    }

  if (copiedAtt->numberOfItems() != 2)
    {
    std::cerr << "Copy attribute produced unexpected results" << "\n";
    std::cerr << logger.convertToString();
    return -2;
    }

  smtk::attribute::StringItemPtr myCopiedStrings =
    copiedAtt->findString("myStrings");

  if (myCopiedStrings->numberOfValues() != myStrings->numberOfValues())
    {
    std::cerr << "Unexpected number of string values: "<<myCopiedStrings->numberOfValues()<<" vs "<<myStrings->numberOfValues()<<"\n";
    std::cerr << logger.convertToString();
    return -2;
    }

  smtk::attribute::FileItemPtr myCopiedFiles = copiedAtt->findFile("myFiles");

  if (myCopiedFiles->numberOfValues() != myFiles->numberOfValues())
    {
    std::cerr << "Unexpected number of file values: "<<myCopiedFiles->numberOfValues()<<" vs "<<myFiles->numberOfValues()<<"\n";
    std::cerr << logger.convertToString();
    return -2;
    }
  }

  return 0;
}
