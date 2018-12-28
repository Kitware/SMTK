//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef smtk_session_oscillator_CreateModel_h
#define smtk_session_oscillator_CreateModel_h

#include "smtk/session/oscillator/Exports.h"

#include "smtk/model/Model.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace model
{
class Group;
}
}

namespace smtk
{
namespace session
{
namespace oscillator
{

/**\brief Create a oscillator model which includes a oscillator core.
 * The core is a group in smtk world which is only used as a container
 * to hold pin and duct instances. All other properties are stored on the model.
  */
class SMTKOSCILLATORSESSION_EXPORT CreateModel : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::oscillator::CreateModel);
  smtkCreateMacro(CreateModel);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace oscillator
} //namespace session
} // namespace smtk

#endif // smtk_session_oscillator_CreateModel_h
