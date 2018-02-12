//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/OperationLog.h"
#include "smtk/attribute/IntItem.h"

namespace smtk
{
namespace io
{

OperationLog::OperationLog(smtk::model::ManagerPtr /*mgr*/)
// : m_hasFailures(false)
// , m_manager(mgr)
{
  // mgr->observe(smtk::operation::Operation::CREATED_OPERATOR, OperationLog::operatorCreated, this);
}

OperationLog::~OperationLog()
{
  // smtk::model::ManagerPtr mgr = this->m_manager.lock();
  // if (mgr)
  //   mgr->unobserve(smtk::operation::Operation::CREATED_OPERATOR, OperationLog::operatorCreated, this);

  // smtk::operation::OperationPtr watched;
  // WeakOpArray::iterator it;
  // for (it = this->m_watching.begin(); it != this->m_watching.end(); ++it)
  // {
  //   if ((watched = it->lock()))
  //   {
  //     watched->unobserve(
  //       smtk::operation::Operation::WILL_OPERATE, OperationLog::operatorInvoked, this);
  //     watched->unobserve(
  //       smtk::operation::Operation::DID_OPERATE, OperationLog::operatorReturned, this);
  //   }
  // }
}

// bool OperationLog::hasFailures() const
// {
//   return this->m_hasFailures;
// }

// void OperationLog::resetFailures()
// {
//   this->m_hasFailures = false;
// }

// int OperationLog::operatorCreated(
//   smtk::operation::Operation::EventType event, const smtk::operation::Operation& op, void* data)
// {
//   OperationLog* self = reinterpret_cast<OperationLog*>(data);
//   if (!self || event != smtk::operation::Operation::CREATED_OPERATOR)
//     return 0; // Don't stop an operation just because the recorder is unhappy.

//   smtk::operation::OperationPtr oper =
//     smtk::const_pointer_cast<smtk::operation::Operation>(op.shared_from_this());
//   self->m_watching.push_back(oper);
//   oper->observe(smtk::operation::Operation::WILL_OPERATE, OperationLog::operatorInvoked, self);
//   oper->observe(smtk::operation::Operation::DID_OPERATE, OperationLog::operatorReturned, self);
//   return 0;
// }

// int OperationLog::operatorInvoked(
//   smtk::operation::Operation::EventType event, const smtk::operation::Operation& op, void* data)
// {
//   OperationLog* self = reinterpret_cast<OperationLog*>(data);
//   if (!self)
//     return 0; // Don't stop an operation just because the recorder is unhappy.

//   return self->recordInvocation(event, op);
// }

// int OperationLog::operatorReturned(smtk::operation::Operation::EventType event,
//   const smtk::operation::Operation& op, smtk::operation::Operation::Result r, void* data)
// {
//   OperationLog* self = reinterpret_cast<OperationLog*>(data);
//   if (!self)
//     return 0; // Don't stop an operation just because the recorder is unhappy.

//   self->m_hasFailures |=
//     (r->findInt("outcome")->value(0) == smtk::operation::Operation::OPERATION_FAILED);
//   return self->recordResult(event, op, r);
// }

} // namespace io
} // namespace smtk
