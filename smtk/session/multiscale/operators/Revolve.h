//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_multiscale_Revolve_h
#define __smtk_session_multiscale_Revolve_h

#include "smtk/session/multiscale/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace multiscale
{

/**\brief An operator for revolving a 2-d mesh to generate a 3-d mesh.
 *
 * For the AFRL Materials Phase I Demo, takes the 2-dimensional mesh and
 * revolves it about an axis, generating a 3-dimensional mesh.
 */

class SMTKMULTISCALESESSION_EXPORT Revolve : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::multiscale::Revolve);
  smtkCreateMacro(Revolve);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace multiscale
} // namespace session
} // namespace smtk

#endif // __smtk_session_multiscale_Revolve_h
