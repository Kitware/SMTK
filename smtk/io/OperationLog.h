//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_io_OperationLog_h
#define __smtk_io_OperationLog_h
/*! \file */

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SystemConfig.h"
#include "smtk/model/Events.h"

namespace smtk
{
namespace io
{

/**\brief Log operations run in a session for later adaptation or replay.
  *
  * This class captures all of the operator creation events
  * for a given model manager and invokes pure virtual methods
  * to log the events if they are deemed acceptable by a filter.
  */
class SMTKCORE_EXPORT OperationLog
{
public:
  OperationLog(smtk::model::ManagerPtr mgr);
  virtual ~OperationLog();

  // bool hasFailures() const;
  // void resetFailures();

protected:
  /**\brief Log the invocation of an operator.
    *
    * Subclasses must implement this method.
    * Be aware that this method may not be called for
    * all operators if a filter is in place.
    */
  // virtual int recordInvocation(
  //   smtk::operation::Operation::EventType event, const smtk::operation::Operation& op) = 0;

  /**\brief Log the result of an operator.
    *
    * Subclasses must implement this method.
    * Be aware that this method may not be called for
    * all operators if a filter is in place.
    */
  // virtual int recordResult(smtk::operation::Operation::EventType event,
  //   const smtk::operation::Operation& op, smtk::operation::Operation::Result r) = 0;

  // static int operatorCreated(
  //   smtk::operation::Operation::EventType event, const smtk::operation::Operation& op, void* user);
  // static int operatorInvoked(
  //   smtk::operation::Operation::EventType event, const smtk::operation::Operation& op, void* user);
  // static int operatorReturned(smtk::operation::Operation::EventType event,
  //   const smtk::operation::Operation& op, smtk::operation::Operation::Result r, void* user);

  // typedef std::vector<smtk::operation::WeakOperationPtr> WeakOpArray;

  // bool m_hasFailures;
  // smtk::model::WeakManagerPtr m_manager;
  // WeakOpArray m_watching;
};

} // namespace io
} // namespace smtk

#endif /* __smtk_io_OperationLog_h */
