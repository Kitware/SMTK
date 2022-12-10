//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/CloseModel.h"

#include "smtk/model/Session.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Session.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/resource/Component.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/model/operators/CloseModel_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace model
{

bool CloseModel::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }
  smtk::attribute::ConstReferenceItemPtr modelItem = this->parameters()->associations();
  return modelItem && modelItem->numberOfValues() > 0;
}

CloseModel::Result CloseModel::operateInternal()
{
  // ableToOperate should have verified that model(s) are set
  smtk::attribute::ReferenceItem::Ptr modelItem = this->parameters()->associations();

  smtk::model::Resource::Ptr resource = std::static_pointer_cast<smtk::model::Resource>(
    std::static_pointer_cast<smtk::resource::Component>(modelItem->value())->resource());

  std::set<smtk::resource::ComponentPtr> expunged;
  bool success = true;
  for (auto it = modelItem->begin(); it != modelItem->end(); ++it)
  {
    auto model =
      std::static_pointer_cast<smtk::model::Entity>(*it)->referenceAs<smtk::model::Model>();
    // Auxiliary geometry must be added to the "expunged" attribute so it can be properly closed on
    // the client side. It must be added before we erase the model, or else the auxiliary geometries
    // will not be accessible via the model.
    AuxiliaryGeometries auxs = model.auxiliaryGeometry();
    for (AuxiliaryGeometries::iterator ait = auxs.begin(); ait != auxs.end(); ++ait)
    {
      expunged.insert(ait->component());
    }

    // Similarly, meshes must be added to the "mesh_expunged" attribute.
    auto associatedMeshes = resource->links().linkedFrom(smtk::mesh::Resource::ClassificationRole);
    for (const auto& cit : associatedMeshes)
    {
      auto meshResource = std::dynamic_pointer_cast<smtk::resource::Resource>(cit);
      smtk::resource::Component::Visitor temp = [&](const smtk::resource::ComponentPtr& c) {
        expunged.insert(c);
      };
      meshResource->visit(temp);
    }

    if (!resource->eraseModel(model))
    {
      success = false;
      break;
    }
  }

  Result result = this->createResult(
    success ? smtk::operation::Operation::Outcome::SUCCEEDED
            : smtk::operation::Operation::Outcome::FAILED);

  if (success)
  {
    smtk::attribute::ResourceItem::Ptr modifiedItem = result->findResource("resource modified");
    modifiedItem->appendValue(resource);

    smtk::attribute::ComponentItem::Ptr expungedItem = result->findComponent("expunged");
    for (const auto& e : expunged)
    {
      expungedItem->appendValue(e);
    }
  }
  return result;
}

const char* CloseModel::xmlDescription() const
{
  return CloseModel_xml;
}

} //namespace model
} // namespace smtk
