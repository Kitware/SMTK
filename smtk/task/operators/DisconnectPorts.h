//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_task_DisconnectPorts_h
#define smtk_task_DisconnectPorts_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace task
{

/**\brief Connect a pair of ports so that one task configures another.
  *
  * All ports are owned by tasks. Connecting an output port A owned
  * by task T_A to an input port B owned by task T_B results in
  * task T_B being reconfigured with information from T_A whenever
  * port A is marked as modified.
  *
  * Note that in order to be connected, the upstream port (A) must
  * generate configuration data that is a superset of what the
  * downstream port (B) accepts.
  */
class SMTKCORE_EXPORT DisconnectPorts : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::task::DisconnectPorts);
  smtkSuperclassMacro(smtk::operation::XMLOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkCreateMacro(DisconnectPorts);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_DisconnectPorts_h
