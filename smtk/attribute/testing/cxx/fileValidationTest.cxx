//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/System.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include <iostream>

namespace
{
const char* testInput =
  "<?xml version=\"1.0\" encoding=\"utf-8\" ?>                                   "
  "<SMTK_AttributeSystem Version=\"2\">                                          "
  "  <Definitions>                                                               "
  "    <AttDef Type=\"att1\" BaseType=\"\">                                      "
  "      <ItemDefinitions>                                                       "
  "        <File Name=\"multipleExtensions\" NumberOfRequiredValues=\"1\"        "
  "          FileFilters=\"Ext1 (*.ex1);;Ext2 or 3 (*.ex2 *.ex3)\">              "
  "        </File>                                                               "
  "        <File Name=\"anyExtension\" NumberOfRequiredValues=\"1\"              "
  "          FileFilters=\"Ext1 (*.ex1);;Ext2 or 3 (*.ex2 *.ex3);;All (*.*)\">   "
  "        </File>                                                               "
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
  smtk::attribute::SystemPtr system = smtk::attribute::System::create();
  smtk::io::Logger logger;
  smtk::io::AttributeReader reader;

  if (reader.readContents(system, testInput, logger))
  {
    std::cerr << "Encountered Errors while reading input data\n";
    std::cerr << logger.convertToString();
    return -2;
  }

  std::vector<smtk::attribute::AttributePtr> atts;
  system->attributes(atts);
  if (atts.size() != 1)
  {
    std::cerr << "Unexpected number of attributes: " << atts.size() << "\n";
    std::cerr << logger.convertToString();
    return -2;
  }
  smtk::attribute::AttributePtr att = atts[0];

  {
    smtk::attribute::FileItemPtr file = att->findFile("multipleExtensions");

    bool ok = file->setValue("");
    if (ok != false)
    {
      std::cerr << "Empty ile name was not properly filtered.\n";
      return 1;
    }

    ok = file->setValue("foo");
    if (ok != false)
    {
      std::cerr << "File name with no extension was not properly filtered.\n";
      return 1;
    }

    ok = file->setValue("foo.ex1");
    if (ok != true)
    {
      std::cerr << "File name with appropriate extension was rejected.\n";
      return 1;
    }

    ok = file->setValue("foo.ex2");
    if (ok != true)
    {
      std::cerr << "File name with appropriate extension was rejected.\n";
      return 1;
    }

    ok = file->setValue("foo.ex4");
    if (ok != false)
    {
      std::cerr << "File name with incorrect extension was accepted.\n";
      return 1;
    }
  }

  {
    smtk::attribute::FileItemPtr file = att->findFile("anyExtension");

    bool ok = file->setValue("");
    if (ok != false)
    {
      std::cerr << "Empty ile name was not properly filtered.\n";
      return 1;
    }

    ok = file->setValue("foo");
    if (ok != true)
    {
      std::cerr << "File name with no extension was rejected.\n";
      return 1;
    }

    ok = file->setValue("foo.ex2");
    if (ok != true)
    {
      std::cerr << "File name with appropriate extension was rejected.\n";
      return 1;
    }
  }

  return 0;
}
