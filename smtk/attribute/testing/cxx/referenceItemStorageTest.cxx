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
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include <iostream>

namespace
{
const char* testInput =
  "<?xml version=\"1.0\" encoding=\"utf-8\" ?>                                   "
  "<SMTK_AttributeResource Version=\"3\">                                        "
  "  <Definitions>                                                               "
  "    <AttDef Type=\"att\" BaseType=\"\">                                       "
  "      <ItemDefinitions>                                                       "
  "	   <Resource Name=\"holdReference\"  NumberOfRequiredValues=\"1\"        "
  "                  HoldReference=\"1\">                                        "
  "          <Accepts>                                                           "
  "            <Resource Name=\"smtk::attribute::Resource\"/>                    "
  "          </Accepts>                                                          "
  "	   </Resource>                                                           "
  "        <Resource Name=\"dropReference\"  NumberOfRequiredValues=\"1\"        "
  "                  HoldReference=\"0\">                                        "
  "          <Accepts>                                                           "
  "            <Resource Name=\"smtk::attribute::Resource\"/>                    "
  "          </Accepts>                                                          "
  "        </Resource>                                                           "
  "      </ItemDefinitions>                                                      "
  "    </AttDef>                                                                 "
  "  </Definitions>                                                              "
  "</SMTK_AttributeResource>                                                     ";
}

int main()
{
  // Create an attribute resource as described by the above preamble
  smtk::attribute::ResourcePtr resource = smtk::attribute::Resource::create();
  {
    smtk::io::Logger logger;
    smtk::io::AttributeReader reader;

    if (reader.readContents(resource, testInput, logger))
    {
      std::cerr << "Encountered Errors while reading input data\n";
      std::cerr << logger.convertToString();
      return -2;
    }
  }

  // Create instances of resource items that
  smtk::attribute::AttributePtr att = resource->createAttribute("att");
  smtk::attribute::ResourceItemPtr holdReference = att->findResource("holdReference");
  smtk::attribute::ResourceItemPtr dropReference = att->findResource("dropReference");

  {
    smtk::attribute::ResourcePtr heldResource = smtk::attribute::Resource::create();

    holdReference->setValue(heldResource);
    dropReference->setValue(heldResource);

    if (holdReference->value() == nullptr || dropReference->value() == nullptr)
    {
      std::cerr << "Encountered Errors while assigning reference item\n";
      return -2;
    }
  }

  if (holdReference->value() == nullptr || dropReference->value() == nullptr)
  {
    std::cerr << "Resource was not held by reference item\n";
    return -1;
  }

  holdReference->reset();

  if (dropReference->value() != nullptr)
  {
    std::cerr << "Resource was incorrectly held by reference item\n";
    return -1;
  }

  return 0;
}
