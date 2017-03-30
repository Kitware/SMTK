//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_multiscale_Session_h
#define __smtk_session_multiscale_Session_h

#include "smtk/bridge/multiscale/Exports.h"

#include "smtk/bridge/mesh/Session.h"
#include "smtk/model/EntityRef.h"

namespace smtk
{
namespace model
{

class ArrangementHelper;
}
}

namespace smtk
{
namespace bridge
{
namespace multiscale
{

class SMTKMULTISCALESESSION_EXPORT Session : public smtk::bridge::mesh::Session
{
public:
  smtkTypeMacro(smtk::bridge::multiscale::Session);
  smtkSuperclassMacro(smtk::bridge::mesh::Session);
  smtkSharedFromThisMacro(smtk::model::Session);
  smtkCreateMacro(smtk::bridge::multiscale::Session);
  smtkDeclareModelingKernel();

  virtual ~Session();

protected:
  friend class Operator;
  friend class Dream3DOperator;

  typedef smtk::model::SessionInfoBits SessionInfoBits;

  Session();

private:
  Session(const Session&);        // Not implemented.
  void operator=(const Session&); // Not implemented.
};

} // namespace multiscale
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_multiscale_Session_h
