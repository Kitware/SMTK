//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================

#include "smtk/markup/operators/TagIndividual.h"

#include "smtk/markup/Ontology.h"
#include "smtk/markup/OntologyIdentifier.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/io/Logger.h"

#include "smtk/markup/TagIndividual_xml.h"

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

TagIndividual::Result TagIndividual::operateInternal()
{
  auto result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  auto created = result->findComponent("created");
  auto modified = result->findComponent("modified");

  auto assocs = this->parameters()->associations();
  auto ontologyName = this->parameters()->findString("ontology")->value();
  auto nodeName = this->parameters()->findString("name")->value();
  auto nodeURL = this->parameters()->findString("url")->value();

  // Identify the markup resource to edit:
  smtk::markup::Resource* resource = nullptr;
  for (const auto& obj : *assocs)
  {
    if (auto* assoc = dynamic_cast<smtk::markup::Component*>(obj.get()))
    {
      resource = dynamic_cast<smtk::markup::Resource*>(assoc->parentResource());
      if (resource)
      {
        break;
      }
    }
  }
  if (!resource)
  {
    smtkErrorMacro(this->log(), "No markup resource to target.");
    return result;
  }

  // Find an ontology named as specified or create one if it doesn't exist yet.
  std::string filterQuery = "'smtk::markup::Ontology' [string{'name'='" + ontologyName + "'}]";
  auto nodes = resource->filter(filterQuery);
  smtk::markup::Ontology::Ptr ontology;
  bool createdOntology = false;
  if (nodes.empty())
  {
    ontology = resource->createNode<smtk::markup::Ontology>();
    ontology->setName(ontologyName);
    created->appendValue(ontology);
    createdOntology = true;
  }
  else if (nodes.size() > 1)
  {
    smtkErrorMacro(
      this->log(),
      "More than 1 ontology named \"" << ontologyName
                                      << "\" found "
                                         "("
                                      << nodes.size() << " in fact).");
    return result;
  }
  else
  {
    ontology = std::dynamic_pointer_cast<smtk::markup::Ontology>(*nodes.begin());
    if (!ontology)
    {
      smtkErrorMacro(this->log(), "Uberon ontology node could not be cast to type.");
      return result;
    }
  }

  // Find or create an ontology identifier with the given name and URL (unless they are both empty).
  filterQuery = "'smtk::markup::OntologyIdentifier' [string{'name'='" + nodeName + "'}]";
  OntologyIdentifier::Ptr oid;
  nodes = resource->filter(filterQuery);
  bool createdOntologyIdentifier = false;
  if (nodes.empty())
  {
    oid = resource->createNode<smtk::markup::OntologyIdentifier>();
    oid->setName(nodeName);
    oid->setOntologyId(nodeURL);
    oid->parent().connect(ontology);
    created->appendValue(oid);
    createdOntologyIdentifier = true;
  }
  else if (nodes.size() > 1)
  {
    smtkErrorMacro(
      this->log(),
      "More than 1 ontology identifier named \"" << nodeName
                                                 << "\" found "
                                                    "("
                                                 << nodes.size() << " in fact).");
    return result;
  }
  else
  {
    oid = std::dynamic_pointer_cast<smtk::markup::OntologyIdentifier>(*nodes.begin());
    if (!oid)
    {
      smtkErrorMacro(this->log(), "Ontology identifier could not be cast to type.");
      return result;
    }
  }

  // Now we have everything we need to mark objects, we know the oid will be modified.
  if (!createdOntologyIdentifier)
  {
    modified->appendValue(oid);
  }
  else if (createdOntology && createdOntologyIdentifier)
  {
    modified->appendValue(ontology);
  }

  // Connect all the associated components to the ontology identifier (or remove any ontology if null).
  for (const auto& obj : *assocs)
  {
    if (auto* assoc = dynamic_cast<smtk::markup::Component*>(obj.get()))
    {
      if (!assoc->incoming<arcs::OntologyIdentifiersToIndividuals>().connect(oid))
      {
        smtkErrorMacro(
          this->log(),
          "Could not mark \"" << assoc->name() << "\" (" << assoc->typeName()
                              << ")"
                                 " with ontology identifier.");
        // TODO: Remove created oid, ontology as needed; clear modified item.
        return result;
      }
    }
  }

  // TODO: If no associations, disconnect all nodes from the ontology identifier and remove it.
  //       (But how would we know what resource to target?)

  result->findInt("outcome")->setValue(static_cast<int>(TagIndividual::Outcome::SUCCEEDED));
  return result;
}

const char* TagIndividual::xmlDescription() const
{
  return TagIndividual_xml;
}

} // namespace markup
} // namespace smtk
