//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_oscillator_Read_h
#define smtk_session_oscillator_Read_h

#include "smtk/session/oscillator/Resource.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace oscillator
{

/**\brief Read an SMTK oscillator model file.
  */
class SMTKOSCILLATORSESSION_EXPORT Read : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::oscillator::Read);
  smtkCreateMacro(Read);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
  void markModifiedResources(Result&) override;
};

SMTKOSCILLATORSESSION_EXPORT smtk::resource::ResourcePtr read(
  const std::string&,
  const std::shared_ptr<smtk::common::Managers>&);

} // namespace oscillator
} // namespace session
} // namespace smtk

#endif // smtk_session_oscillator_Read_h
