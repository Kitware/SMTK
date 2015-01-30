//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_discrete_SplitFaceOperator_h
#define __smtk_session_discrete_SplitFaceOperator_h

#include "smtk/bridge/discrete/discreteSessionExports.h"
#include "smtk/model/Operator.h"
#include "vtkSplitOperator.h"
#include "vtkNew.h"

namespace smtk {
  namespace bridge {
    namespace discrete {

class Session;

class SMTKDISCRETESESSION_EXPORT SplitFaceOperator : public smtk::model::Operator
{
public:
  smtkTypeMacro(SplitFaceOperator);
  smtkCreateMacro(SplitFaceOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  SplitFaceOperator();
  virtual smtk::model::OperatorResult operateInternal();
  Session* discreteSession() const;
  int fetchCMBFaceId() const;

  vtkNew<vtkSplitOperator> m_op;
};

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_SplitFaceOperator_h
