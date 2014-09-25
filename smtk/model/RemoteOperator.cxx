//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/RemoteOperator.h"

#include "smtk/model/DefaultBridge.h"
#include "smtk/model/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

using smtk::attribute::IntItem;
using smtk::attribute::IntItemPtr;

namespace smtk {
  namespace model {

/// Return the name of this operator.
std::string RemoteOperator::name() const
{
  return this->m_name;
}

/// Set the name of this operator (should only be called by ImportJSON::ofOperator).
RemoteOperator::Ptr RemoteOperator::setName(const std::string& opName)
{
  this->m_name = opName;
  return shared_from_this();
}

/**\brief Call the bridge's delegate method to see if the
  *       remote process can perform the operation.
  */
bool RemoteOperator::ableToOperate()
{
  DefaultBridge* fwdBridge = dynamic_cast<DefaultBridge*>(this->bridge());
  if (!fwdBridge)
    return false;

  return fwdBridge->ableToOperateDelegate(shared_from_this());
}

OperatorResult RemoteOperator::operateInternal()
{
  DefaultBridge* fwdBridge = dynamic_cast<DefaultBridge*>(this->bridge());
  if (!fwdBridge)
    return this->createResult(OPERATION_FAILED);

  return fwdBridge->operateDelegate(shared_from_this());
}

  } // model namespace
} // smtk namespace

smtkImplementsModelOperator(
  smtk::model::RemoteOperator,
  RemoteOperator,
  "remote op",
  NULL /* no XML specification */,
  smtk::model::DefaultBridge);
