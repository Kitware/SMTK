//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_oscillator_Export_h
#define smtk_session_oscillator_Export_h

#include "smtk/operation/XMLOperation.h"
#include "smtk/session/oscillator/Exports.h"

namespace smtk
{
namespace session
{
namespace oscillator
{

/**\brief Export simulation configuration files for the SENSEI oscillator mini-app.
  *
  */
class SMTKOSCILLATORSESSION_EXPORT Export : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::oscillator::Export);
  smtkCreateMacro(Export);
  smtkSuperclassMacro(smtk::operation::XMLOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

SMTKOSCILLATORSESSION_EXPORT bool exportResource(const smtk::resource::ResourcePtr&);

} // namespace oscillator
} // namespace session
} // namespace smtk

#endif // smtk_session_oscillator_Export_h
