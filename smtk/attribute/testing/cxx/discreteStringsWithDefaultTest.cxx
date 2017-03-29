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
#include "smtk/attribute/StringItem.h"
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
"	<String Name=\"normalString\" Extensible=\"0\"                         "
"               NumberOfRequiredValues=\"1\">                                  "
"         <DefaultValue>normal</DefaultValue>                                  "
"	</String>                                                              "
"	<String Name=\"discreteString\" Extensible=\"0\"                       "
"               NumberOfRequiredValues=\"1\">                                  "
"          <DiscreteInfo DefaultIndex=\"1\">                                   "
"            <Value Enum=\"String1\">String1</Value>                           "
"            <Value Enum=\"String2\">String2</Value>                           "
"            <Value Enum=\"String3\">String3</Value>                           "
"            <Value Enum=\"String4\">String4</Value>                           "
"            <Value Enum=\"String5\">String5</Value>                           "
"            <Value Enum=\"String6\">String6</Value>                           "
"          </DiscreteInfo>                                                     "
"	</String>                                                              "
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

  att->findString("normalString")->setToDefault();

  std::cout<<"Default value: "
           <<att->findString("normalString")->defaultValue()<<std::endl;
  std::cout<<"Value: "
           <<att->findString("normalString")->value()<<std::endl;
  std::cout<<"Value as string: "
           <<att->findString("normalString")->valueAsString()<<std::endl;

  if (att->findString("normalString")->defaultValue() != "normal")
    {
    return 1;
    }
  if (att->findString("normalString")->value() != "normal")
    {
    return 1;
    }
  if (att->findString("normalString")->valueAsString() != "normal")
    {
    return 1;
    }

  att->findString("discreteString")->setToDefault();

  std::cout<<"Default value: "
           <<att->findString("discreteString")->defaultValue()<<std::endl;
  std::cout<<"Value: "
           <<att->findString("discreteString")->value()<<std::endl;
  std::cout<<"Value as string: "
           <<att->findString("discreteString")->valueAsString()<<std::endl;

  if (att->findString("discreteString")->defaultValue() != "String2")
    {
    return 1;
    }
  if (att->findString("discreteString")->value() != "String2")
    {
    return 1;
    }
  if (att->findString("discreteString")->valueAsString() != "String2")
    {
    return 1;
    }

  return 0;
}
