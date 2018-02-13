//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/bridge/mesh/Session.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Face.h"
#include "smtk/model/Model.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/Volume.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{

Session::Session()
{
}

Topology* Session::topology(smtk::model::Model& model)
{
  std::vector<Topology>::iterator it = find_if(this->m_topologies.begin(), this->m_topologies.end(),
    [&](const Topology& t) { return t.m_collection->entity() == model.entity(); });
  return (it == this->m_topologies.end() ? nullptr : &(*it));
}

smtk::model::SessionInfoBits Session::transcribeInternal(
  const smtk::model::EntityRef& entityRef, SessionInfoBits requestedInfo, int depth)
{
  // Start with a null return value
  SessionInfoBits actual = smtk::model::SESSION_NOTHING;

  // Find the topology associated with the entity
  const smtk::bridge::mesh::Topology* topology = nullptr;
  for (const Topology& top : this->m_topologies)
  {
    if (entityRef.entity() == top.m_collection->entity())
    {
      // The entity is the model associated with this topology
      topology = &top;
      break;
    }
    else if (top.m_elements.find(entityRef.entity()) != top.m_elements.end())
    {
      // The entity is an element associated with this topology
      topology = &top;
      break;
    }
  }

  // If we don't find the associated topology, return the null value
  if (topology == nullptr)
  {
    return actual;
  }

  const Topology::Element& topologyElement = topology->m_elements.at(entityRef.entity());

  // create a mutable copy of our entity ref, since the original one is const
  smtk::model::EntityRef mutableEntityRef(entityRef);
  int dimension = topologyElement.m_dimension;

  // if the entity ref is not already valid, we populate it
  if (!mutableEntityRef.isValid())
  {
    // Check if the entityRef is for the model. If it is, then its UUID will be
    // identical to that of the collection that represents the model.
    if (mutableEntityRef.entity() == topology->m_collection->entity())
    {
      // We have a model. We insert the model using the manager API. It takes
      // the UUID of the model, its parametric dimension and its embedded
      // dimension. Additionally, we denote that this model is discrete.
      this->manager()->insertModel(mutableEntityRef.entity(), dimension, dimension);
      mutableEntityRef.setIntegerProperty(SMTK_GEOM_STYLE_PROP, smtk::model::DISCRETE);
    }
    else
    {
      // We have a component of the model. We insert the component using the
      // manager API.
      switch (dimension)
      {
        case 0:
          this->manager()->insertVertex(mutableEntityRef.entity());
          break;
        case 1:
          this->manager()->insertEdge(mutableEntityRef.entity());
          break;
        case 2:
          this->manager()->insertFace(mutableEntityRef.entity());
          break;
        case 3:
          this->manager()->insertVolume(mutableEntityRef.entity());
          break;
        default:
          return actual;
      }
    }
    // We have assigned the model entity type, so we record that we have done so
    actual |= smtk::model::SESSION_ENTITY_TYPE;
  }
  else
  {
    // If the entity is valid, is there any reason to refresh it?
    // Perhaps we want additional information transcribed?
    if (this->danglingEntities().find(mutableEntityRef) == this->danglingEntities().end())
    {
      return smtk::model::SESSION_EVERYTHING;
    }
  }

  // We now have a valid entity in our model system.
  // Next, we assign relations between entities.
  if (requestedInfo & (smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS))
  {
    for (auto&& childId : topologyElement.m_children)
    {
      smtk::model::EntityRef childEntityRef(this->manager(), childId);
      if (!childEntityRef.isValid())
      {
        // Remove this child from the list of dangling entities
        this->declareDanglingEntity(childEntityRef, 0);
        // Recursively run on this child.
        this->transcribeInternal(childEntityRef, requestedInfo, depth < 0 ? depth : depth - 1);
      }
      mutableEntityRef.findOrAddRawRelation(childEntityRef);
      childEntityRef.findOrAddRawRelation(mutableEntityRef);
    }
    // We have assigned the model relations, so we record that we have done so
    actual |= (smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS);
  }

  if (requestedInfo & smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS)
  {
    // TODO
    actual |= smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS;
  }

  // Meshes are visible without an associated tessellation, so there is nothing
  // to do here
  if (requestedInfo & smtk::model::SESSION_TESSELLATION)
  {
    actual |= smtk::model::SESSION_TESSELLATION;
  }

  if (requestedInfo & smtk::model::SESSION_PROPERTIES)
  {
    // Models generated from a mesh set naturally have the same properties as
    // mesh sets. We query the model entity's associated mesh set for domain,
    // Dirichlet and Neumann properties.

    if (topologyElement.m_mesh.size() == 1)
    {
      // To avoid elements that only partially contain the properties in question,
      // we check that the associated meshset contains exactly one property and
      // that the property completely spans the meshset.
      std::vector<smtk::mesh::Domain> domains = topologyElement.m_mesh.domains();
      std::vector<smtk::mesh::Dirichlet> dirichlets = topologyElement.m_mesh.dirichlets();
      std::vector<smtk::mesh::Neumann> neumanns = topologyElement.m_mesh.neumanns();

      if (domains.size() == 1 && topologyElement.m_mesh.size() == 1)
      {
        mutableEntityRef.setIntegerProperty("pedigree id", domains[0].value());
        std::stringstream s;
        s << m_facade["domain"] << " " << domains[0].value();
        mutableEntityRef.setName(s.str());
      }
      else if (dirichlets.size() == 1 && topologyElement.m_mesh.size() == 1)
      {
        mutableEntityRef.setIntegerProperty("pedigree id", dirichlets[0].value());
        std::stringstream s;
        s << m_facade["dirichlet"] << " " << dirichlets[0].value();
        mutableEntityRef.setName(s.str());
      }
      else if (neumanns.size() == 1 && topologyElement.m_mesh.size() == 1)
      {
        mutableEntityRef.setIntegerProperty("pedigree id", neumanns[0].value());
        std::stringstream s;
        s << m_facade["neumann"] << " " << neumanns[0].value();
        mutableEntityRef.setName(s.str());
      }
    }

    actual |= smtk::model::SESSION_PROPERTIES;
  }

  return actual;
}
}
}
}
