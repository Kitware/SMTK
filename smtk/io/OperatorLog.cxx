//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/OperatorLog.h"
#include "smtk/model/Operator.h"
#include "smtk/attribute/IntItem.h"

using namespace smtk::model;

namespace smtk {
  namespace io {

OperatorLog::OperatorLog(smtk::model::ManagerPtr mgr)
  : m_hasFailures(false), m_manager(mgr)
{
  mgr->observe(smtk::model::CREATED_OPERATOR, OperatorLog::operatorCreated, this);
}

OperatorLog::~OperatorLog()
{
  smtk::model::ManagerPtr mgr = this->m_manager.lock();
  if (mgr)
    mgr->unobserve(smtk::model::CREATED_OPERATOR, OperatorLog::operatorCreated, this);

  smtk::model::OperatorPtr watched;
  WeakOpSet::iterator it;
  for (it = this->m_watching.begin(); it != this->m_watching.end(); ++it)
    {
    if ((watched = it->lock()))
      {
      watched->unobserve(smtk::model::WILL_OPERATE, OperatorLog::operatorInvoked, this);
      watched->unobserve(smtk::model::DID_OPERATE, OperatorLog::operatorReturned, this);
      }
    }
}

bool OperatorLog::hasFailures() const
{
  return this->m_hasFailures;
}

void OperatorLog::resetFailures()
{
  this->m_hasFailures = false;
}

int OperatorLog::operatorCreated(
  smtk::model::OperatorEventType event,
  const smtk::model::Operator& op,
  void* data)
{
  OperatorLog* self = reinterpret_cast<OperatorLog*>(data);
  if (!self || event != CREATED_OPERATOR)
    return 0; // Don't stop an operation just because the recorder is unhappy.

  OperatorPtr oper = smtk::const_pointer_cast<Operator>(op.shared_from_this());
  self->m_watching.insert(oper);
  oper->observe(smtk::model::WILL_OPERATE, OperatorLog::operatorInvoked, self);
  oper->observe(smtk::model::DID_OPERATE, OperatorLog::operatorReturned, self);
  return 0;
}

int OperatorLog::operatorInvoked(
  smtk::model::OperatorEventType event,
  const smtk::model::Operator& op,
  void* data)
{
  OperatorLog* self = reinterpret_cast<OperatorLog*>(data);
  if (!self)
    return 0; // Don't stop an operation just because the recorder is unhappy.

  return self->recordInvocation(event, op);
}

int OperatorLog::operatorReturned(
  smtk::model::OperatorEventType event,
  const smtk::model::Operator& op,
  smtk::model::OperatorResult r,
  void* data)
{
  OperatorLog* self = reinterpret_cast<OperatorLog*>(data);
  if (!self)
    return 0; // Don't stop an operation just because the recorder is unhappy.

  self->m_hasFailures |=
    (r->findInt("outcome")->value(0) == smtk::model::OPERATION_FAILED);
  return self->recordResult(event, op, r);
}

  } // namespace io
} // namespace smtk
