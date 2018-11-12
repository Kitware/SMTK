// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef smtk_session_rgg_Delete_h
#define smtk_session_rgg_Delete_h

#include "smtk/session/rgg/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace rgg
{

/**\brief Delete rgg entities(Ex pin, cell)
    */
class SMTKRGGSESSION_EXPORT Delete : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::rgg::Delete);
  smtkCreateMacro(Delete);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace rgg
} //namespace session
} // namespace smtk

#endif // smtk_session_rgg_Delete_h
