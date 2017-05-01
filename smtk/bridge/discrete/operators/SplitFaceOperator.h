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

#include "smtk/bridge/discrete/Operator.h"
#include "vtkNew.h"
#include "vtkSplitOperator.h"

namespace smtk
{
namespace bridge
{
namespace discrete
{

class SMTKDISCRETESESSION_EXPORT SplitFaceOperator : public Operator
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
  int fetchCMBFaceId() const;
  int fetchCMBCellId(const smtk::attribute::ModelEntityItemPtr& entItem, int idx) const;

  vtkNew<vtkSplitOperator> m_op;
};

} // namespace discrete
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_SplitFaceOperator_h
