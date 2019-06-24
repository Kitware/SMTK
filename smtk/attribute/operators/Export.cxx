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

#include "smtk/attribute/Export_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

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
    std::dynamic_pointer_cast<smtk::attribute::Resource>(resourceItem->objectValue());

  // Export the attribute resource to file.
  smtk::io::AttributeWriter writer;
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
}
}
