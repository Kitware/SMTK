//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/operators/Export.h"

#include "smtk/attribute/operators/Export_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include <vector>

namespace smtk
{
namespace attribute
{

Export::Result Export::operateInternal()
{
  // Access the file name.
  std::string outputfile = this->parameters()->findFile("filename")->value();

  // Access the attribute resource to export.
  auto resourceItem = this->parameters()->associations();
  smtk::attribute::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::attribute::Resource>(resourceItem->value());

  // Get the attribute collection option
  std::vector<DefinitionPtr> defnList;
  auto collectionItem = this->parameters()->findGroup("attribute-collection");
  if ((collectionItem != nullptr) && collectionItem->isEnabled())
  {
    auto typesItem = collectionItem->findAs<StringItem>("types");
    if (typesItem != nullptr)
    {
      for (std::size_t i = 0; i < typesItem->numberOfValues(); ++i)
      {
        std::string attType = typesItem->value(i);
        auto defn = resource->findDefinition(attType);
        if (defn == nullptr)
        {
          smtkErrorMacro(this->log(), "Did not find attribute type \"" << attType << "\"");
          return this->createResult(smtk::operation::Operation::Outcome::FAILED);
        }
        defnList.push_back(defn);
      }
      if (defnList.empty())
      {
        smtkErrorMacro(this->log(), "No attribute types specified");
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
    }
  } // if (collectionItem enabled)

  // Export the attribute resource to file.
  smtk::io::AttributeWriter writer;
  if (!defnList.empty())
  {
    writer.treatAsLibrary(defnList);
  }
  if (writer.write(resource, outputfile, log()))
  {
    smtkErrorMacro(log(), "Encountered errors while writing \"" << outputfile << "\".");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  return this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

const char* Export::xmlDescription() const
{
  return Export_xml;
}
} // namespace attribute
} // namespace smtk
