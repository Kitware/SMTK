//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/operators/MarkModified.h"

#include "smtk/operation/operators/MarkModified_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace operation
{

MarkModified::Result MarkModified::operateInternal()
{
  auto params = this->parameters();
  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  auto modIn = params->associations();
  auto modOut = result->findResource("resources");

  // Copy the inputs to the output.
  modOut->setValues(modIn->begin(), modIn->end());
  return result;
}

// Do not print a message to the console.
void MarkModified::generateSummary(Operation::Result& /*unused*/) {}

const char* MarkModified::xmlDescription() const
{
  return MarkModified_xml;
}
} // namespace operation
} // namespace smtk
