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
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/CloseModel_xml.h"

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
  smtk::attribute::ModelEntityItem::Ptr modelItem = this->parameters()->findModelEntity("model");
  return modelItem && modelItem->numberOfValues() > 0;
}

CloseModel::Result CloseModel::operateInternal()
{
  // ableToOperate should have verified that model(s) are set
  smtk::attribute::ModelEntityItem::Ptr modelItem = this->parameters()->findModelEntity("model");

  smtk::model::Manager::Ptr resource =
    std::static_pointer_cast<smtk::model::Manager>(modelItem->value().component()->resource());

  EntityRefArray expunged;
  smtk::mesh::MeshSets expungedMeshes;
  bool success = true;
  for (EntityRefArray::const_iterator mit = modelItem->begin(); mit != modelItem->end(); ++mit)
  {
    // Auxiliary geometry must be added to the "expunged" attribute so it can be properly closed on
    // the client side. It must be added before we erase the model, or else the auxiliary geometries
    // will not be accessible via the model.
    AuxiliaryGeometries auxs = mit->as<smtk::model::Model>().auxiliaryGeometry();
    for (AuxiliaryGeometries::iterator ait = auxs.begin(); ait != auxs.end(); ++ait)
    {
      expunged.push_back(*ait);
    }

    // Similarly, meshes must be added to the "mesh_expunged" attribute.
    for (auto cit : resource->meshes()->associatedCollections(mit->as<smtk::model::Model>()))
    {
      expungedMeshes.insert(cit->meshes());
    }

    if (!resource->eraseModel(*mit))
    {
      success = false;
      break;
    }
    expunged.push_back(*mit);
  }

  Result result = this->createResult(success ? smtk::operation::Operation::Outcome::SUCCEEDED
                                             : smtk::operation::Operation::Outcome::FAILED);

  if (success)
  {
    smtk::attribute::ComponentItem::Ptr expungedItem = result->findComponent("expunged");
    for (auto& e : expunged)
    {
      expungedItem->appendValue(e.component());
    }

    result->findMesh("mesh_expunged")->appendValues(expungedMeshes);
  }
  return result;
}

const char* CloseModel::xmlDescription() const
{
  return CloseModel_xml;
}

} //namespace model
} // namespace smtk
