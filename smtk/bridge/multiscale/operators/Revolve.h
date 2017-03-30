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

#include "smtk/bridge/multiscale/Operator.h"
#include "vtkObject.h"

namespace smtk
{
namespace bridge
{
namespace multiscale
{

/**\brief An operator for revolving a 2-d mesh to generate a 3-d mesh.
 *
 * For the AFRL Materials Phase I Demo, takes the 2-dimensional mesh and
 * revolves it about an axis, generating a 3-dimensional mesh.
 */

class SMTKMULTISCALESESSION_EXPORT Revolve : public Operator
{
public:
  smtkTypeMacro(Revolve);
  smtkCreateMacro(Revolve);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

} // namespace multiscale
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_multiscale_Revolve_h
