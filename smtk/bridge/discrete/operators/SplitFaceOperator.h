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
#include "smtk/bridge/discrete/Resource.h"
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
  smtkSharedFromThisMacro(smtk::operation::NewOp);
  smtkSuperclassMacro(Operator);

  bool ableToOperate() override;

protected:
  SplitFaceOperator();
  Result operateInternal() override;
  const char* xmlDescription() const override;
  int fetchCMBFaceId(smtk::bridge::discrete::Resource::Ptr& resource) const;
  int fetchCMBCellId(smtk::bridge::discrete::Resource::Ptr& resource,
    const smtk::attribute::ModelEntityItemPtr& entItem, int idx) const;

  vtkNew<vtkSplitOperator> m_op;
};

} // namespace discrete
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_SplitFaceOperator_h
