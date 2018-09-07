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

#include "smtk/session/discrete/Operation.h"

namespace smtk
{
namespace session
{
namespace discrete
{

class SMTKDISCRETESESSION_EXPORT RemoveModel : public Operation
{
public:
  smtkTypeMacro(smtk::session::discrete::RemoveModel);
  smtkCreateMacro(RemoveModel);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace discrete
} // namespace session
} // namespace smtk

#endif // __smtk_session_discrete_RemoveModel_h
