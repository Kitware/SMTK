//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================

#include "smtk/markup/operators/DumpGraph.h"

#include "smtk/markup/Ontology.h"
#include "smtk/markup/OntologyIdentifier.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/Logger.h"

#include "smtk/markup/operators/DumpGraph_xml.h"

// On Windows MSVC 2015+, something is included that defines
// a macro named ERROR to be 0. This causes smtkErrorMacro()
// to expand into garbage (because smtk::io::Logger::ERROR
// gets expanded to smtk::io::Logger::0).
#ifdef ERROR
#undef ERROR
#endif

namespace smtk
{
namespace markup
{

DumpGraph::Result DumpGraph::operateInternal()
{
  auto result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  auto created = result->findComponent("created");
  auto modified = result->findComponent("modified");

  auto assocs = this->parameters()->associations();
  auto filename = this->parameters()->findFile("filename")->value();

  // Identify the markup resource to edit:
  smtk::markup::Resource* resource = nullptr;
  for (const auto& obj : *assocs)
  {
    resource = dynamic_cast<smtk::markup::Resource*>(obj.get());
    if (resource)
    {
      break;
    }
  }
  if (!resource)
  {
    smtkErrorMacro(this->log(), "No markup resource to dump.");
    return result;
  }

  resource->dump(filename, "text/vnd.graphviz");
  result->findInt("outcome")->setValue(static_cast<int>(DumpGraph::Outcome::SUCCEEDED));
  return result;
}

const char* DumpGraph::xmlDescription() const
{
  return DumpGraph_xml;
}

} // namespace markup
} // namespace smtk
