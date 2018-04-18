//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/bridge/rgg/Session.h"

namespace smtk
{
namespace bridge
{
namespace rgg
{
typedef smtk::model::SessionInfoBits SessionInfoBits;

Session::Session()
{
  // TODO: initialize operator collection
  this->initializeOperatorCollection(Session::s_operators);
}

SessionInfoBits Session::transcribeInternal(
  const model::EntityRef& entity, SessionInfoBits requestedInfo, int depth)
{
  (void)entity;
  (void)depth;
  (void)requestedInfo;
  return smtk::model::SESSION_EVERYTHING;
}
}
}
}

#include "smtk/bridge/rgg/Session_json.h" // For Session_json
smtkImplementsModelingKernel(
  SMTKRGGSESSION_EXPORT, rgg, Session_json, smtk::model::SessionHasNoStaticSetup,
  smtk::bridge::rgg::Session, true /* inherit "universal" operators */
  );
