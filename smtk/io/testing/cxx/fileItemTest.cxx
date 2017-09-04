//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>
#include <string>

namespace sa = smtk::attribute;
namespace si = smtk::io;

int testFileItemSerialization()
{
  // Create simple attribute collection with file item assigned 1 value
  auto collection = sa::Collection::create();
  sa::DefinitionPtr attdef = collection->createDefinition("file-item-test");
  sa::FileItemDefinitionPtr itemdef = sa::FileItemDefinition::New("file-item");
  attdef->addItemDefinition(itemdef);

  sa::AttributePtr att = collection->createAttribute("file-item-att", attdef);
  sa::FileItemPtr item = att->findFile("file-item");
  item->setValue(__FILE__);

  // Serialize
  si::Logger logger;
  si::AttributeWriter writer;
  std::string content;
  writer.writeContents(collection, content, logger);

  // Deserialize
  sa::CollectionPtr readbackCollection = sa::Collection::create();
  si::AttributeReader reader;
  bool err = reader.readContents(readbackCollection, content, logger);
  if (err)
  {
    std::cout << "ERROR\n";
    std::cout << logger.convertToString();
    return -1;
  }

  // Check file item
  sa::AttributePtr readbackAtt = readbackCollection->findAttribute("file-item-att");
  sa::FileItemPtr readbackItem = readbackAtt->findFile("file-item");
  test(readbackItem->value() == __FILE__, "file item did not match");

  return 0;
}

int main(int /*argc*/, char* /*argv*/ [])
{
  testFileItemSerialization();
  return 0;
}
