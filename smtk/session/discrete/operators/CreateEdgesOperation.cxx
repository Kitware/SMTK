//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "CreateEdgesOperation.h"

#include "smtk/session/discrete/Resource.h"
#include "smtk/session/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

#include "smtk/session/discrete/kernel/Model/vtkModel.h"
#include "smtk/session/discrete/kernel/Model/vtkModelFace.h"
#include "smtk/session/discrete/kernel/Model/vtkModelItem.h"
#include "smtk/session/discrete/kernel/Model/vtkModelItemIterator.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"

#include "CreateEdgesOperation_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace session
{

namespace discrete
{

CreateEdgesOperation::CreateEdgesOperation() = default;

bool CreateEdgesOperation::ableToOperate()
{
  smtk::model::Model model = this->parameters()->associations()->valueAs<smtk::model::Entity>();
  if (!model.isValid())
  {
    return false;
  }
  smtk::session::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::discrete::Resource>(model.component()->resource());
  vtkDiscreteModelWrapper* modelWrapper =
    resource->discreteSession()->findModelEntity(model.entity());
  if (!modelWrapper)
  {
    return false;
  }

  bool operable = true;
  // verify that faces in the model do not have edges already.
  vtkModelItemIterator* iter = modelWrapper->GetModel()->NewIterator(vtkModelFaceType);
  for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
  {
    vtkModelFace* face = vtkModelFace::SafeDownCast(iter->GetCurrentItem());
    if (face && face->GetNumberOfModelEdges() > 0)
    {
      operable = false;
      break;
    }
  }
  iter->Delete();

  return operable;
}

CreateEdgesOperation::Result CreateEdgesOperation::operateInternal()
{
  smtk::model::EntityRef inModel =
    this->parameters()->associations()->valueAs<smtk::model::Entity>();

  smtk::session::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::discrete::Resource>(inModel.component()->resource());

  vtkDiscreteModelWrapper* modelWrapper =
    resource->discreteSession()->findModelEntity(inModel.entity());
  if (!modelWrapper)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  m_op->SetShowEdges(1);
  m_op->Operate(modelWrapper);
  bool ok = m_op->GetOperateSucceeded() != 0;
  Result result = this->createResult(ok ? smtk::operation::Operation::Outcome::SUCCEEDED
                                        : smtk::operation::Operation::Outcome::FAILED);

  if (ok)
  {
    // this will remove and re-add the model so that the model topology and all
    // relationships will be reset properly.
    resource->discreteSession()->retranscribeModel(inModel);
    smtk::model::EntityRefArray modEnts;
    modEnts.push_back(inModel);

    // also mark all model faces are modified since there are likely new edges created
    smtk::common::UUID faceUID;
    vtkModelItemIterator* iter = modelWrapper->GetModel()->NewIterator(vtkModelFaceType);
    for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
    {
      vtkModelFace* face = vtkModelFace::SafeDownCast(iter->GetCurrentItem());
      faceUID = resource->discreteSession()->findOrSetEntityUUID(face);
      modEnts.push_back(smtk::model::EntityRef(resource, faceUID));
    }
    iter->Delete();

    result->findComponent("tess_changed")->setValue(inModel.component());
    smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");
    for (auto m : modEnts)
    {
      modified->appendValue(m.component());
    }
  }

  return result;
}

const char* CreateEdgesOperation::xmlDescription() const
{
  return CreateEdgesOperation_xml;
}

} // namespace discrete
} // namespace session
} // namespace smtk
