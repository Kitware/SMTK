//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_cgm_Reflect_h
#define __smtk_session_cgm_Reflect_h

#include "smtk/session/cgm/Operation.h"

namespace smtk
{
namespace session
{
namespace cgm
{

class SMTKCGMSESSION_EXPORT Reflect : public Operation
{
public:
  smtkTypeMacro(Reflect);
  smtkCreateMacro(Reflect);
  smtkSharedFromThisMacro(Operation);
  smtkDeclareModelOperation();

protected:
  Result operateInternal() override;
};

} // namespace cgm
} //namespace session
} // namespace smtk

#endif // __smtk_session_cgm_Reflect_h
