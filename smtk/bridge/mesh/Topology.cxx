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
#include "smtk/bridge/mesh/Topology.h"

#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/ForEachTypes.h"
#include "smtk/mesh/MeshSet.h"

#include <numeric>

namespace smtk {
namespace bridge {
namespace mesh {

namespace
{
typedef std::vector<std::pair<smtk::mesh::MeshSet,
                      Topology::Element*> > ElementShells;

class AddFreeElements : public smtk::mesh::MeshForEach
{
public:
  AddFreeElements(Topology* topology,
                  Topology::Element* root) : m_topology(topology),
                                             m_root(root),
                                             m_shells(nullptr),
                                             m_dimension(-1) {}

  void setElementShells(ElementShells* shells) { this->m_shells = shells; }
  void setDimension(int dimension) { this->m_dimension = dimension; }

  void forMesh(smtk::mesh::MeshSet& singleMesh)
  {
    // Each free mesh is an element. It gets a unique id and has the model as
    // its parent.
    smtk::common::UUID id = (this->m_topology->m_collection->modelManager()->
                             unusedUUID());
    // Assign the unique id to the mesh
    singleMesh.setModelEntityId(id);
    // add the unique id as a child of the model
    this->m_root->m_children.push_back(id);
    // construct an element for the mesh, insert it into the topology's map
    // with its id as the key, and store its shell as a pair along with a
    // pointer to its associated Element (for use in extracting bound elements).
    Topology::Element* element = &this->m_topology->m_elements.insert(
      std::make_pair(id, Topology::Element(this->m_dimension))).first->second;
    if (this->m_shells)
      {
      smtk::mesh::MeshSet shell = singleMesh.extractShell();
      if (!shell.is_empty())
        {
        for(std::size_t i = 0; i < shell.size(); i++)
          {
          if (!shell.subset(i).is_empty())
            {
            this->m_shells->push_back(std::make_pair(shell.subset(i), element));
            }
          }
        }
      }
  }

protected:
  Topology* m_topology;
  Topology::Element* m_root;
  ElementShells* m_shells;
  int m_dimension;
};

struct AddBoundElements
{
  AddBoundElements(Topology* topology) : m_topology(topology) {}

  void setElementShells(ElementShells* shells) { this->m_shells = shells; }
  void setDimension(int dimension) { this->m_dimension = dimension; }

  void operator()(ElementShells::iterator start, ElementShells::iterator end)
  {
    std::vector<ElementShells::iterator> activeShells;

    ElementShells::iterator seed = start;

    while (seed != end)
    {
      activeShells.push_back(seed++);

      while (!activeShells.empty())
        {
        smtk::mesh::MeshSet m = activeShells.back()->first;
        for (ElementShells::iterator j = seed; j != end; ++j)
          {
          smtk::mesh::CellSet cs = smtk::mesh::set_intersect(m.cells(),
                                                             j->first.cells());
          if (!cs.is_empty())
            {
            m = this->m_topology->m_collection->createMesh(cs);
            activeShells.push_back(j);
            }

          if (this->m_dimension == 2 && activeShells.size() == 2)
            {
            break;
            }
          }

        if (!m.is_empty())
          {
          // We have an intersection, so we must now process it. We start by
          // creating a new id for it.
          smtk::common::UUID id = (this->m_topology->m_collection->
                                   modelManager()->unusedUUID());

          // Next, we remove the intersection from the contributing mesh sets
          // and add the new id as a child of these sets.
          for (auto&& shell : activeShells)
            {
            smtk::mesh::MeshSet tmp =
              this->m_topology->m_collection->createMesh(
                smtk::mesh::set_difference(shell->first.cells(), m.cells()));
            this->m_topology->m_collection->removeMeshes(shell->first);
            shell->first = tmp;
            shell->second->m_children.push_back(id);
            }

          // We then register the intersection witih the new id
          m.setModelEntityId(id);
          // finally, we insert it as an element into the topology. If
          // necessary, we store its shell for the bound element calculation of
          // lower dimension
          Topology::Element* element = &this->m_topology->m_elements.insert(
            std::make_pair(id, Topology::Element(this->m_dimension))).
            first->second;
          if (this->m_shells)
            {
            this->m_shells->push_back(std::make_pair(m.extractShell(),
                                                     element));
            }
          }
        if (activeShells.size() == 1)
          {
          activeShells.pop_back();
          }
        else
          {
          activeShells.erase(std::next(activeShells.begin()),
                             activeShells.end());
          }
        }
      }
  }

  Topology* m_topology;
  ElementShells* m_shells;
  int m_dimension;
};

}

Topology::Topology(smtk::mesh::CollectionPtr collection) :
  m_collection(collection)
{
  // Insert the collection as the top-level element representing the model
  Element* model =
    &(this->m_elements.insert(std::make_pair(collection->entity(),
                                             Element())).first->second);

  // Extract single meshes as volumes and grab their shells
  ElementShells volumeShells;
  ElementShells faceShells;
  ElementShells edgeShells;

  ElementShells* elementShells[4] = { nullptr, &edgeShells, &faceShells,
                                      &volumeShells };

  AddFreeElements addFreeElements(this, model);
  // all volume meshes are considered to be free elements with the model as
  // their parent
    {
    int dimension = 3;
    addFreeElements.setElementShells(elementShells[dimension]);
    addFreeElements.setDimension(dimension);
    smtk::mesh::for_each(collection->meshes(
                           static_cast<smtk::mesh::DimensionType>(dimension)),
                         addFreeElements);
    }

  AddBoundElements addBoundElements(this);
  for (int dimension = 2; dimension >= 0; dimension--)
    {
    addFreeElements.setElementShells(elementShells[dimension]);
    addFreeElements.setDimension(dimension);

    smtk::mesh::MeshSet allMeshes = collection->meshes(
      static_cast<smtk::mesh::DimensionType>(dimension));
    smtk::mesh::MeshSet boundMeshes;

    for (auto&& shell : *elementShells[dimension+1])
      {
      boundMeshes.append(shell.first);
      }
    smtk::mesh::MeshSet freeMeshes = m_collection->createMesh(
      smtk::mesh::set_difference(allMeshes.cells(), boundMeshes.cells()));
    smtk::mesh::for_each(freeMeshes, addFreeElements);

    addBoundElements.setElementShells(elementShells[dimension]);
    addBoundElements.setDimension(dimension);
    addBoundElements(elementShells[dimension + 1]->begin(),
                     elementShells[dimension + 1]->end());
    }

  if (false)
    {
    std::size_t count[4] = {0,0,0,0};
    for (auto&& i : this->m_elements)
      if (i.second.m_dimension >= 0 && i.second.m_dimension <= 3)
        count[i.second.m_dimension]++;

    std::cout<<count[3]<<" volumes"<<std::endl;
    std::cout<<count[2]<<" faces"<<std::endl;
    std::cout<<count[1]<<" edges"<<std::endl;
    std::cout<<count[0]<<" vertices"<<std::endl;
    }
}

}
}
}
