//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_rgg_RemoveMaterial_h
#define __smtk_session_rgg_RemoveMaterial_h

#include "smtk/session/rgg/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace rgg
{

/**\brief Remove a custom material from rgg session
  */
class SMTKRGGSESSION_EXPORT RemoveMaterial : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::rgg::RemoveMaterial);
  smtkCreateMacro(RemoveMaterial);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace rgg
} //namespace session
} // namespace smtk

#endif // __smtk_session_rgg_RemoveMaterial_h
