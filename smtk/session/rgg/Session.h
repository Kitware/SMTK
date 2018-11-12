//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_rgg_Session_h
#define __smtk_session_rgg_Session_h

#include "smtk/session/rgg/Exports.h"

#include "smtk/model/Session.h"

namespace smtk
{
namespace session
{
namespace rgg
{

class SMTKRGGSESSION_EXPORT Session : public smtk::model::Session
{
public:
  smtkTypeMacro(Session);
  smtkSuperclassMacro(smtk::model::Session);
  smtkSharedFromThisMacro(smtk::model::Session);
  smtkCreateMacro(smtk::model::Session);
  typedef smtk::model::SessionInfoBits SessionInfoBits;

  virtual ~Session() {}

protected:
  Session();

  SessionInfoBits transcribeInternal(
    const smtk::model::EntityRef& entity, SessionInfoBits requestedInfo, int depth = -1) override;
};

} // namespace rgg
} // namespace session
} // namespace smtk

#endif // __smtk_session_rgg_Session_h
