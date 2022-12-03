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
#include "smtk/markup/ontology/Source.h"
#include "smtk/markup/operators/Delete.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

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
  auto expunged = result->findComponent("expunged");

  auto assocs = this->parameters()->associations();
  auto delItem = this->parameters()->findComponent("tagsToRemove");
  auto addGroup = this->parameters()->findGroup("tagsToAdd");

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

  std::size_t gg = addGroup->numberOfGroups();
  for (std::size_t ii = 0; ii < gg; ++ii)
  {
    bool createdIdentifier = false;
    auto nodeName = addGroup->findAs<smtk::attribute::StringItem>(ii, "name")->value();
    auto ontologyName = addGroup->findAs<smtk::attribute::StringItem>(ii, "ontology")->value();
    auto nodeURL = addGroup->findAs<smtk::attribute::StringItem>(ii, "url")->value();
    auto ontologyId = this->findOrCreateTag(
      resource, nodeName, ontologyName, nodeURL, created, modified, createdIdentifier);
    if (!ontologyId)
    {
      return result;
    }
    if (this->tagNodes(ontologyId, assocs, modified) > 0 && !createdIdentifier)
    {
      modified->appendValue(ontologyId);
    }
  }

  for (const auto& ontObj : *delItem)
  {
    if (auto ontologyId = std::dynamic_pointer_cast<smtk::markup::OntologyIdentifier>(ontObj))
    {
      this->untagNodes(ontologyId, assocs, modified, expunged);
    }
  }

  result->findInt("outcome")->setValue(static_cast<int>(TagIndividual::Outcome::SUCCEEDED));
  return result;
}

smtk::markup::OntologyIdentifier::Ptr TagIndividual::findOrCreateTag(
  smtk::markup::Resource* resource,
  const std::string& nodeName,
  const std::string& ontologyName,
  const std::string& nodeURL,
  const smtk::attribute::ComponentItem::Ptr& created,
  const smtk::attribute::ComponentItem::Ptr& modified,
  bool& createdIdentifier)
{
  smtk::markup::OntologyIdentifier::Ptr tag;
  // Find an ontology named as specified or create one if it doesn't exist yet.
  auto ontos = resource->findByName<std::vector<smtk::markup::Ontology*>>(ontologyName);
  smtk::markup::Ontology::Ptr ontology;
  bool createdOntology = false;
  if (ontos.empty())
  {
    ontology = resource->createNode<smtk::markup::Ontology>();
    ontology->setName(ontologyName);
    const auto& source = smtk::markup::ontology::Source::findByName(ontologyName);
    if (!source.url().empty())
    {
      ontology->setUrl(source.url());
    }
    else
    {
      smtkWarningMacro(
        this->log(),
        "Unregistered ontology \""
          << ontologyName
          << "\" requested."
             "Either the ontology name is incorrect or a plugin is missing.");
    }
    created->appendValue(ontology);
    createdOntology = true;
  }
  else if (ontos.size() > 1)
  {
    smtkErrorMacro(
      this->log(),
      "More than 1 ontology named \"" << ontologyName
                                      << "\" found "
                                         "("
                                      << ontos.size() << " in fact).");
    return tag;
  }
  else
  {
    ontology = std::dynamic_pointer_cast<smtk::markup::Ontology>(ontos.front()->shared_from_this());
    if (!ontology)
    {
      smtkErrorMacro(this->log(), "Uberon ontology node could not be cast to type.");
      return tag;
    }
  }

  // Find or create an ontology identifier with the given name and URL (unless they are both empty).
  auto nodes = resource->findByName<std::vector<smtk::markup::OntologyIdentifier*>>(nodeName);
  createdIdentifier = false;
  if (nodes.empty())
  {
    tag = resource->createNode<smtk::markup::OntologyIdentifier>();
    tag->setName(nodeName);
    tag->setOntologyId(nodeURL);
    tag->parent().connect(ontology);
    created->appendValue(tag);
    createdIdentifier = true;
  }
  else if (nodes.size() > 1)
  {
    smtkErrorMacro(
      this->log(),
      "More than 1 ontology identifier named \"" << nodeName
                                                 << "\" found "
                                                    "("
                                                 << nodes.size() << " in fact).");
    return tag;
  }
  else
  {
    tag = std::dynamic_pointer_cast<smtk::markup::OntologyIdentifier>(
      nodes.front()->shared_from_this());
    if (!tag)
    {
      smtkErrorMacro(this->log(), "Ontology identifier could not be cast to type.");
      return tag;
    }
  }

  // Now we have everything we need to mark objects, we know the tag will be modified.
  if (tag && !createdIdentifier)
  {
    modified->appendValue(tag);
  }
  else if (createdOntology && createdIdentifier)
  {
    modified->appendValue(ontology);
  }

  return tag;
}

std::size_t TagIndividual::tagNodes(
  const smtk::markup::OntologyIdentifier::Ptr& tag,
  const smtk::attribute::ReferenceItem::Ptr& assocs,
  const smtk::attribute::ComponentItem::Ptr& modified)
{
  std::size_t numTagged = 0;
  for (const auto& assoc : *assocs)
  {
    if (auto node = std::dynamic_pointer_cast<smtk::markup::Component>(assoc))
    {
      if (tag->subjects().connect(node))
      {
        modified->appendValue(node);
        ++numTagged;
      }
    }
  }
  return numTagged;
}

std::size_t TagIndividual::untagNodes(
  const smtk::markup::OntologyIdentifier::Ptr& tag,
  const smtk::attribute::ReferenceItem::Ptr& assocs,
  const smtk::attribute::ComponentItem::Ptr& modified,
  const smtk::attribute::ComponentItem::Ptr& expunged)
{
  std::size_t numUntagged = 0;
  for (const auto& assoc : *assocs)
  {
    if (auto node = std::dynamic_pointer_cast<smtk::markup::Component>(assoc))
    {
      if (tag->subjects().disconnect(node))
      {
        modified->appendValue(node);
        ++numUntagged;
      }
    }
  }
  if (tag->subjects().empty())
  {
    // Delete tag and add to expunged.
    expunged->appendValue(tag);
    auto delOp = smtk::markup::Delete::create();
    delOp->parameters()->associations()->appendValue(tag);
    delOp->operate(TagIndividual::Key{});
  }
  else if (numUntagged)
  {
    modified->appendValue(tag);
  }
  return numUntagged;
}

const char* TagIndividual::xmlDescription() const
{
  return TagIndividual_xml;
}

} // namespace markup
} // namespace smtk
