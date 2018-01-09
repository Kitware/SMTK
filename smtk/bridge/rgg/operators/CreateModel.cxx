//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/CreateModel.h"

#include "smtk/bridge/rgg/Session.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/rgg/CreateModel_xml.h"

namespace smtk
{
namespace bridge
{
namespace rgg
{

smtk::model::OperatorResult CreateModel::operateInternal()
{
  smtk::model::OperatorResult result;
  smtk::bridge::rgg::SessionPtr sess = this->activeSession();
  smtk::model::Manager::Ptr mgr;
  if (sess)
  {
    // If a name was specified, use it. Or make one up.
    smtk::attribute::StringItem::Ptr nameItem = this->findString("name");
    std::string modelName;
    if (nameItem && nameItem->isEnabled())
    {
      modelName = nameItem->value(0);
    }

    mgr = sess->manager();
    smtk::model::Model model = mgr->addModel(/* par. dim. */ 3, /* emb. dim. */ 3, modelName);
    model.setSession(smtk::model::SessionRef(mgr, sess->sessionId()));
    if (modelName.empty())
    {
      model.assignDefaultName();
    }

    result = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);
    this->addEntityToResult(result, model, CREATED);
    // QUESTION: set SMTK_GEOM_STYLE_PROP?
    model.setIntegerProperty(SMTK_GEOM_STYLE_PROP, smtk::model::DISCRETE);
  }

  if (!result)
  {
    result = this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  return result;
}

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::CreateModel, rgg_create_model,
  "create model", CreateModel_xml, smtk::bridge::rgg::Session);
