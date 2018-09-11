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

#include "smtk/session/discrete/Operation.h"
#include "smtk/session/discrete/Resource.h"
#include "vtkNew.h"
#include "vtkSplitOperation.h"

namespace smtk
{
namespace session
{
namespace discrete
{

class SMTKDISCRETESESSION_EXPORT SplitFaceOperation : public Operation
{
public:
  smtkTypeMacro(smtk::session::discrete::SplitFaceOperation);
  smtkCreateMacro(SplitFaceOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

  bool ableToOperate() override;

protected:
  SplitFaceOperation();
  Result operateInternal() override;
  const char* xmlDescription() const override;
  int fetchCMBFaceId(smtk::session::discrete::Resource::Ptr& resource) const;
  int fetchCMBCellId(smtk::session::discrete::Resource::Ptr& resource,
    const smtk::attribute::ReferenceItemPtr& entItem, int idx) const;

  vtkNew<vtkSplitOperation> m_op;
};

} // namespace discrete
} // namespace session
} // namespace smtk

#endif // __smtk_session_discrete_SplitFaceOperation_h
