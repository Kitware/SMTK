//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_discrete_BathymetryOperator_h
#define __smtk_session_discrete_BathymetryOperator_h

#include "smtk/model/Operator.h"
#include "smtk/bridge/discrete/Exports.h"
#include "smtk/bridge/discrete/operation/vtkCMBModelPointsOperator.h"
#include "vtkNew.h"

namespace smtk {
  namespace bridge {
    namespace discrete {

class Session;

class SMTKDISCRETESESSION_EXPORT BathymetryOperator : public smtk::model::Operator
{
public:
  smtkTypeMacro(BathymetryOperator);
  smtkCreateMacro(BathymetryOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  BathymetryOperator();
  virtual smtk::model::OperatorResult operateInternal();
  Session* discreteSession() const;

  vtkNew<vtkCMBModelPointsOperator> m_op;
};

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_BathymetryOperator_h
