//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_multiscale_Session_h
#define smtk_session_multiscale_Session_h

#include "smtk/session/multiscale/Exports.h"

#include "smtk/model/EntityRef.h"
#include "smtk/session/mesh/Session.h"

namespace smtk
{
namespace model
{

class ArrangementHelper;
}
} // namespace smtk

namespace smtk
{
namespace session
{
namespace multiscale
{

class SMTKMULTISCALESESSION_EXPORT Session : public smtk::session::mesh::Session
{
public:
  smtkTypeMacro(smtk::session::multiscale::Session);
  smtkSuperclassMacro(smtk::session::mesh::Session);
  smtkSharedFromThisMacro(smtk::model::Session);
  smtkCreateMacro(smtk::session::multiscale::Session);

  ~Session() override;

protected:
  friend class Operation;
  friend class Dream3DOperation;

  typedef smtk::model::SessionInfoBits SessionInfoBits;

  Session();
};

} // namespace multiscale
} // namespace session
} // namespace smtk

#endif // smtk_session_multiscale_Session_h
