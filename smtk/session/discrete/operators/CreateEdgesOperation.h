//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_discrete_CreateEdgesOperation_h
#define __smtk_session_discrete_CreateEdgesOperation_h

#include "smtk/session/discrete/Operation.h"
#include "vtkCreateModelEdgesOperation.h"
#include "vtkNew.h"

namespace smtk
{
namespace session
{
namespace discrete
{

class Session;

class SMTKDISCRETESESSION_EXPORT CreateEdgesOperation : public Operation
{
public:
  smtkTypeMacro(smtk::session::discrete::CreateEdgesOperation);
  smtkCreateMacro(CreateEdgesOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

  bool ableToOperate() override;

protected:
  CreateEdgesOperation();
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;

  vtkNew<vtkCreateModelEdgesOperation> m_op;
};

} // namespace discrete
} // namespace session
} // namespace smtk

#endif // __smtk_session_discrete_CreateEdgesOperation_h
