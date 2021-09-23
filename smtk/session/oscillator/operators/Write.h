//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_oscillator_Write_h
#define smtk_session_oscillator_Write_h

#include "smtk/session/oscillator/Resource.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace oscillator
{

/**\brief Write a CMB oscillator model file.
  */
class SMTKOSCILLATORSESSION_EXPORT Write : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::oscillator::Write);
  smtkCreateMacro(Write);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
  void markModifiedResources(Result&) override;
};

SMTKOSCILLATORSESSION_EXPORT bool write(
  const smtk::resource::ResourcePtr&,
  const std::shared_ptr<smtk::common::Managers>&);

} // namespace oscillator
} // namespace session
} // namespace smtk

#endif // smtk_session_oscillator_Write_h
