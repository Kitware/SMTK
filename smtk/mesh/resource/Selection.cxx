//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/resource/Selection.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/common/UUIDGenerator.h"

#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/operators/DeleteMesh.h"

#include "smtk/operation/Manager.h"

namespace
{
class SelectionContainer
{
public:
  SelectionContainer(const std::shared_ptr<smtk::resource::Component>& component,
    const std::weak_ptr<smtk::operation::Manager>& weakManager)
    : m_selection(std::static_pointer_cast<smtk::mesh::Selection>(component->shared_from_this()))
    , m_weakManager(weakManager)
  {
  }

  SelectionContainer(const std::shared_ptr<smtk::resource::Component>& component)
    : SelectionContainer(component, std::weak_ptr<smtk::operation::Manager>())
  {
  }

  ~SelectionContainer()
  {
    std::cout << "delete container " << this << std::endl;
    std::cout << "selection use count starts at " << m_selection.use_count() << std::endl;
    {
      smtk::mesh::DeleteMesh::Ptr deleteMesh;
      if (auto operationManager = m_weakManager.lock())
      {
        deleteMesh = operationManager->create<smtk::mesh::DeleteMesh>();
        deleteMesh->parameters()->associate(m_selection);
        operationManager->launchers()(deleteMesh);
      }
      else
      {
        deleteMesh = smtk::mesh::DeleteMesh::create();
        deleteMesh->parameters()->associate(m_selection);
        deleteMesh->operate();
      }

      std::cout << "selection use count now " << m_selection.use_count() << std::endl;
    }
    std::cout << "selection use count finally " << m_selection.use_count() << std::endl;
    std::cout << "for the record, the selection's raw pointer is " << m_selection.get()
              << std::endl;
  }

  const std::shared_ptr<smtk::mesh::Selection>& selection() const { return m_selection; }

private:
  std::shared_ptr<smtk::mesh::Selection> m_selection;
  std::weak_ptr<smtk::operation::Manager> m_weakManager;
};
}

namespace smtk
{
namespace mesh
{

Selection::Selection(const smtk::mesh::CellSet& cellset)
  : Component(cellset.resource(), smtk::common::UUIDGenerator::instance().random())
  , m_cells(cellset.range())
{
}

Selection::~Selection()
{
  std::cout << "delete selection " << this << std::endl;
}

std::shared_ptr<Selection> Selection::create(const smtk::mesh::CellSet& cellset)
{
  return create(cellset, std::weak_ptr<smtk::operation::Manager>());
}

std::shared_ptr<Selection> Selection::create(
  const smtk::mesh::CellSet& cellset, const std::weak_ptr<smtk::operation::Manager>& weakManager)
{
  std::shared_ptr<smtk::resource::Component> tmp(new Selection(cellset));
  std::shared_ptr<SelectionContainer> selectionContainer(new SelectionContainer(tmp, weakManager));

  std::shared_ptr<smtk::resource::Component> shared(
    selectionContainer, selectionContainer->selection().get());

  return std::static_pointer_cast<smtk::mesh::Selection>(shared);
}

std::string Selection::name() const
{
  return "selection";
}

const smtk::mesh::MeshSet Selection::mesh() const
{
  smtk::mesh::MeshSet meshSet = Component::mesh();
  if (meshSet.isValid() == false)
  {
    if (smtk::mesh::Resource::Ptr resource =
          std::dynamic_pointer_cast<smtk::mesh::Resource>(this->resource()))
    {
      return resource->createMesh(smtk::mesh::CellSet(resource, m_cells));
    }
  }
  return meshSet;
}

smtk::mesh::MeshSet Selection::mesh()
{
  smtk::mesh::MeshSet meshSet = Component::mesh();
  if (meshSet.isValid() == false)
  {
    if (smtk::mesh::Resource::Ptr resource =
          std::dynamic_pointer_cast<smtk::mesh::Resource>(this->resource()))
    {
      return resource->createMesh(smtk::mesh::CellSet(resource, m_cells), this->id());
    }
  }
  return meshSet;
}
}
}
