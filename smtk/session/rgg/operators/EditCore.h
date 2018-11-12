//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_rgg_EditCore_h
#define __smtk_session_rgg_EditCore_h

#include "smtk/session/rgg/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace rgg
{

/**\brief Edit a rgg core. What happens beneath the cover is that smtk is editing
 * properties stored on the model for simplicity purpose since one rgg model
 * can only have one core.
  */
class SMTKRGGSESSION_EXPORT EditCore : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::rgg::EditCore);
  smtkCreateMacro(EditCore);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace rgg
} //namespace session
} // namespace smtk

#endif // __smtk_session_rgg_EditCore_h
