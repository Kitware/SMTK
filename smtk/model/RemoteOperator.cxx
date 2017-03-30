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

#include "smtk/model/DefaultSession.h"
#include "smtk/model/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

using smtk::attribute::IntItem;
using smtk::attribute::IntItemPtr;

namespace smtk
{
namespace model
{

/// Return the name of this operator.
std::string RemoteOperator::name() const
{
  return this->m_name;
}

/// Set the name of this operator (should only be called by LoadJSON::ofOperator).
RemoteOperator::Ptr RemoteOperator::setName(const std::string& opName)
{
  this->m_name = opName;
  return shared_from_this();
}

/**\brief Call the session's delegate method to see if the
  *       remote process can perform the operation.
  */
bool RemoteOperator::ableToOperate()
{
  DefaultSession* fwdSession = dynamic_cast<DefaultSession*>(this->session());
  if (!fwdSession)
    return false;

  return fwdSession->ableToOperateDelegate(shared_from_this());
}

OperatorResult RemoteOperator::operateInternal()
{
  DefaultSession* fwdSession = dynamic_cast<DefaultSession*>(this->session());
  if (!fwdSession)
    return this->createResult(OPERATION_FAILED);

  return fwdSession->operateDelegate(shared_from_this());
}

} // model namespace
} // smtk namespace

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::RemoteOperator, RemoteOperator,
  "remote op", NULL /* no XML specification */, smtk::model::DefaultSession);
