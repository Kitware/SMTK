//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_task_AddDependency_h
#define smtk_task_AddDependency_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace task
{

/**\brief Add a dependency between a pair of tasks.
  *
  * Given tasks A and B, this makes task B dependent on task A.
  * If task B has strict dependencies (i.e., areDependenciesStrict()
  * returns true), then B is unavailable until A is completed.
  * Otherwise, B may not be completed until A is completable but
  * the availability of B is unaffected by A.
  */
class SMTKCORE_EXPORT AddDependency : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::task::AddDependency);
  smtkSuperclassMacro(smtk::operation::XMLOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkCreateMacro(AddDependency);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_AddDependency_h
