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

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"

using namespace smtk::model;

namespace smtk
{
namespace model
{

bool CloseModel::ableToOperate()
{
  if (!this->ensureSpecification())
    return false;
  smtk::attribute::ModelEntityItem::Ptr modelItem = this->specification()->findModelEntity("model");
  return modelItem && modelItem->numberOfValues() > 0;
}

smtk::model::OperatorResult CloseModel::operateInternal()
{
  // ableToOperate should have verified that model(s) are set
  smtk::attribute::ModelEntityItem::Ptr modelItem = this->specification()->findModelEntity("model");

  smtk::model::OperatorPtr removeModel = this->session()->op("remove model");
  if (removeModel)
  {
    for (EntityRefArray::const_iterator mit = modelItem->begin(); mit != modelItem->end(); ++mit)
      removeModel->specification()->associateEntity(*mit);
    return removeModel->operate();
  }

  EntityRefArray expunged;
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

    if (!this->manager()->eraseModel(*mit))
    {
      success = false;
      break;
    }
    expunged.push_back(*mit);
  }

  OperatorResult result = this->createResult(success ? OPERATION_SUCCEEDED : OPERATION_FAILED);

  if (success)
    result->findModelEntity("expunged")->setValues(expunged.begin(), expunged.end());
  return result;
}

} //namespace model
} // namespace smtk

#include "smtk/model/CloseModel_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::CloseModel, close_model, "close model",
  CloseModel_xml, smtk::model::Session);
