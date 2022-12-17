//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/mesh/operators/Merge.h"

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/Session.h"
#include "smtk/session/mesh/Topology.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/common/CompilerInformation.h"

#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/utility/Metrics.h"

#include "smtk/model/Face.h"
#include "smtk/model/Model.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

#include "smtk/session/mesh/operators/Merge_xml.h"

using namespace smtk::model;
using namespace smtk::common;

namespace
{
// When entities are merged, a name for the resulting merged entity is created
// by concatenating the merging entities with " & ". This quickly becomes
// unwieldy when merging large numbers of entities. This value defines the
// maximum number of entities to merge before switching to the naming convention
// of "# entities".
constexpr const std::size_t DescriptionAppendLimit = 5;
} // namespace

namespace smtk
{
namespace session
{
namespace mesh
{

bool Merge::ableToOperate()
{
  if (!this->smtk::operation::XMLOperation::ableToOperate())
  {
    return false;
  }

  // Access the associated model entities
  auto associations = this->parameters()->associations();

  // Access the first input model entity
  auto component = associations->valueAs<smtk::resource::Component>();

  // Access the model resource associated with the first input model entity
  smtk::session::mesh::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::mesh::Resource>(component->resource());

  // Access the model resource's associated topology
  smtk::session::mesh::Topology* topology = resource->session()->topology(resource);

  // Access the dimension of the model entity
  int dimension = topology->m_elements.at(component->id()).m_dimension;

  for (auto it = associations->begin(); it != associations->end(); ++it)
  {
    // All model entities must come from the same resource
    if (
      resource->id() != std::static_pointer_cast<smtk::resource::Component>(*it)->resource()->id())
    {
      return false;
    }

    // Also, all model entities must have the same dimension
    if (dimension != topology->m_elements.at(it->id()).m_dimension)
    {
      return false;
    }
  }

  return true;
}

Merge::Result Merge::operateInternal()
{
  // Access the associated model entities
  auto associations = this->parameters()->associations();

  // Access the model resource associated with all of the input model entities
  smtk::session::mesh::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::mesh::Resource>(
      associations->valueAs<smtk::resource::Component>()->resource());

  // Access the model resource's associated mesh resource
  smtk::mesh::Resource::Ptr meshResource = resource->resource();

  // Access the model resource's associated topology
  smtk::session::mesh::Topology* topology = resource->session()->topology(resource);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
  smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");
  smtk::attribute::ComponentItem::Ptr expunged = result->findComponent("expunged");

  // Create a new id for the merged entity.
  smtk::common::UUID id = resource->unusedUUID();

  smtk::mesh::HandleRange cells;
  std::set<smtk::common::UUID> parents;
  std::set<smtk::common::UUID> children;
  int dimension = -1;
  std::string name;
  std::set<smtk::mesh::MeshSet> toRemove;
  // For each model entity to be merged...
  for (auto it = associations->begin(); it != associations->end(); ++it)
  {
    //...access its associated topology element.
    auto elementIt = topology->m_elements.find(it->id());
    Topology::Element& element = elementIt->second;

    // If This is our first model entity...
    if (dimension == -1)
    {
      //...assign the dimension and parent elements.
      dimension = element.m_dimension;
      parents.insert(element.m_parents.begin(), element.m_parents.end());
    }
    else
    {
      // Otherwise, assign parent elements as the intersection of the current
      // parent list and the parents of the current topology element.
      std::set<smtk::common::UUID> intersection;
      std::set_intersection(
        parents.begin(),
        parents.end(),
        element.m_parents.begin(),
        element.m_parents.end(),
        std::inserter(intersection, intersection.begin()));
      parents = std::move(intersection);
    }

    // Aggregate the cells that comprise the element.
    cells += element.m_mesh.cells().range();

    // Aggregate the element's children.
    children.insert(element.m_children.begin(), element.m_children.end());

    // Access the entity ref associated wit hthe current model entity...
    smtk::model::EntityRef eRef(resource, it->id());

    //...and remove relations to higher- and lower-dimensional entities.
    for (auto parentId = element.m_parents.begin(); parentId != element.m_parents.end(); ++parentId)
    {
      auto parentElementIt = topology->m_elements.find(*parentId);
      Topology::Element& parentElement = parentElementIt->second;
      parentElement.m_children.erase(it->id());
      parentElement.m_children.insert(id);

      smtk::model::EntityRef parentEntityRef(resource, *parentId);
      parentEntityRef.elideRawRelation(eRef);
      eRef.elideRawRelation(parentEntityRef);

      modified->appendValue(parentEntityRef.component());
    }
    for (auto childId = element.m_children.begin(); childId != element.m_children.end(); ++childId)
    {
      auto childElementIt = topology->m_elements.find(*childId);
      Topology::Element& childElement = childElementIt->second;
      childElement.m_parents.erase(it->id());
      childElement.m_parents.insert(id);

      smtk::model::EntityRef childEntityRef(resource, *childId);
      childEntityRef.elideRawRelation(eRef);
      eRef.elideRawRelation(childEntityRef);

      modified->appendValue(childEntityRef.component());
    }

    // Construct a name for the resulting entity.
    if (associations->numberOfValues() <= DescriptionAppendLimit)
    {
      if (!name.empty())
      {
        name += " & ";
      }
      name += eRef.name();
    }
    else
    {
      if (name.empty())
      {
        name = std::to_string(associations->numberOfValues()) + " entities";
      }
    }

    // Add the model entity to the list of expunged components.
    expunged->appendValue(std::static_pointer_cast<smtk::resource::Component>(*it));

    // Remove the model entity from the model resource.
    resource->erase(eRef);

    // Remove the underlying meshset.
    toRemove.insert(element.m_mesh);

    // Finally, remove the topology element.
    topology->m_elements.erase(elementIt);
  }

  smtk::model::EntityRef entityRef(resource, id);
  entityRef.setName(name);

  smtk::mesh::MeshSet mergedMesh =
    meshResource->createMesh(smtk::mesh::CellSet(meshResource, cells));

  // Associate the merged mesh with the merged model entity.
  mergedMesh.setModelEntityId(id);

  // Now that the new mesh is created, we can delete the old meshes without
  // losing the cell and point information.
  for (const auto& mesh : toRemove)
  {
    meshResource->removeMeshes(mesh);
  }

  Topology::Element* element =
    &topology->m_elements.insert(std::make_pair(id, Topology::Element(mergedMesh, id, dimension)))
       .first->second;
  // If there are no common parents for the merged element, assign the
  // encompassing model as its parent.
  if (parents.empty())
  {
    parents.insert(topology->m_modelId);
  }
  element->m_parents.insert(parents.begin(), parents.end());
  element->m_children.insert(children.begin(), children.end());

  // Declare the model as "dangling" so it will be transcribed.
  resource->session()->declareDanglingEntity(entityRef);

  for (const auto& parentId : element->m_parents)
  {
    smtk::model::EntityRef parentEntityRef(resource, parentId);
    resource->session()->declareDanglingEntity(parentEntityRef);
    resource->session()->transcribe(parentEntityRef, smtk::model::SESSION_ENTITY_RELATIONS, false);
  }

  // If we don't call "transcribe" ourselves, it never gets called.
  resource->session()->transcribe(entityRef, smtk::model::SESSION_EVERYTHING, false);

  created->appendValue(entityRef.component());

  return result;
}

const char* Merge::xmlDescription() const
{
  return Merge_xml;
}

} // namespace mesh
} // namespace session
} // namespace smtk
