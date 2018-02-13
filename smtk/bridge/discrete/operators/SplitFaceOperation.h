//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_discrete_SplitFaceOperation_h
#define __smtk_session_discrete_SplitFaceOperation_h

#include "smtk/bridge/discrete/Operation.h"
#include "smtk/bridge/discrete/Resource.h"
#include "vtkNew.h"
#include "vtkSplitOperation.h"

namespace smtk
{
namespace bridge
{
namespace discrete
{

class SMTKDISCRETESESSION_EXPORT SplitFaceOperation : public Operation
{
public:
  smtkTypeMacro(SplitFaceOperation);
  smtkCreateMacro(SplitFaceOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

  bool ableToOperate() override;

protected:
  SplitFaceOperation();
  Result operateInternal() override;
  const char* xmlDescription() const override;
  int fetchCMBFaceId(smtk::bridge::discrete::Resource::Ptr& resource) const;
  int fetchCMBCellId(smtk::bridge::discrete::Resource::Ptr& resource,
    const smtk::attribute::ModelEntityItemPtr& entItem, int idx) const;

  vtkNew<vtkSplitOperation> m_op;
};

} // namespace discrete
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_discrete_SplitFaceOperation_h
