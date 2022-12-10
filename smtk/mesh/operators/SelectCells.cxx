//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/operators/SelectCells.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/operators/SelectCells_xml.h"
#include "smtk/mesh/resource/Selection.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/operation/Manager.h"

namespace smtk
{
namespace mesh
{

SelectCells::Result SelectCells::operateInternal()
{
  // Access the mesh resource.
  smtk::attribute::ReferenceItem::Ptr resourceItem = this->parameters()->associations();
  smtk::mesh::ResourcePtr resource = resourceItem->valueAs<smtk::mesh::Resource>();

  // Access the selected cell ids.
  smtk::mesh::HandleRange cells;
  {
    smtk::attribute::StringItem::Ptr cellIdsItem = this->parameters()->findString("cell ids");
    for (auto cellIt = cellIdsItem->begin(); cellIt != cellIdsItem->end(); ++cellIt)
    {
      cells.insert(std::stoll(*cellIt));
    }
  }

  // Construct a CellSet from the HandleRange.
  smtk::mesh::CellSet cellset(resource, cells);

  // Construct a mesh Selection from the cellset.
  smtk::mesh::Selection::Ptr meshSelection =
    smtk::mesh::Selection::create(cellset, this->m_manager);

  // Access the selection's mesh (this registers the mesh with the underlying
  // resource)
  (void)meshSelection->mesh();

  // Create a new result
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Access the attribute associated with created components
  result->findComponent("created")->appendValue(meshSelection);
  smtk::operation::MarkGeometry().markModified(meshSelection);

  // Return with success
  return result;
}

void SelectCells::generateSummary(Operation::Result& /*unused*/) {}

const char* SelectCells::xmlDescription() const
{
  return SelectCells_xml;
}
} // namespace mesh
} // namespace smtk
