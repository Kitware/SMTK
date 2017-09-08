//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_discrete_CreateEdgesOperator_h
#define __smtk_session_discrete_CreateEdgesOperator_h

#include "smtk/bridge/discrete/Operator.h"
#include "vtkCreateModelEdgesOperator.h"
#include "vtkNew.h"

namespace smtk
{
namespace bridge
{
namespace discrete
{

class Session;

class SMTKDISCRETESESSION_EXPORT CreateEdgesOperator : public Operator
{
public:
  smtkTypeMacro(CreateEdgesOperator);
  smtkCreateMacro(CreateEdgesOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  bool ableToOperate() override;

protected:
  CreateEdgesOperator();
  smtk::model::OperatorResult operateInternal() override;

  vtkNew<vtkCreateModelEdgesOperator> m_op;
};

} // namespace discrete
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_CreateEdgesOperator_h
