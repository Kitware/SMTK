//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_bridge_discrete_CreateEdgesOperator_h
#define __smtk_bridge_discrete_CreateEdgesOperator_h

#include "smtk/bridge/cmb/discreteBridgeExports.h"
#include "smtk/model/Operator.h"
#include "vtkCreateModelEdgesOperator.h"
#include "vtkNew.h"

namespace smtk {
  namespace bridge {
    namespace discrete {

class Bridge;

class SMTKDISCRETEBRIDGE_EXPORT CreateEdgesOperator : public smtk::model::Operator
{
public:
  smtkTypeMacro(CreateEdgesOperator);
  smtkCreateMacro(CreateEdgesOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  CreateEdgesOperator();
  virtual smtk::model::OperatorResult operateInternal();
  Bridge* discreteBridge() const;

  vtkNew<vtkCreateModelEdgesOperator> m_op;
};

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_discrete_CreateEdgesOperator_h
