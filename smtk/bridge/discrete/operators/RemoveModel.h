//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_discrete_RemoveModel_h
#define __smtk_session_discrete_RemoveModel_h

#include "smtk/bridge/discrete/Operator.h"

namespace smtk
{
namespace bridge
{
namespace discrete
{

class SMTKDISCRETESESSION_EXPORT RemoveModel : public Operator
{
public:
  smtkTypeMacro(RemoveModel);
  smtkCreateMacro(RemoveModel);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

} // namespace discrete
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_RemoveModel_h
