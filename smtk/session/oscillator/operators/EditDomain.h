//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_oscillator_EditDomain_h
#define smtk_session_oscillator_EditDomain_h

#include "smtk/session/oscillator/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace oscillator
{

/**\brief Construct a 2- or 3-dimensional uniform grid and its sides.
  */
class SMTKOSCILLATORSESSION_EXPORT EditDomain : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::oscillator::EditDomain);
  smtkCreateMacro(EditDomain);
  smtkSuperclassMacro(smtk::operation::XMLOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};
}
}
}

#endif
