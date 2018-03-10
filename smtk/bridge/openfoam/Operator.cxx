//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/openfoam/Operator.h"
#include "smtk/bridge/openfoam/Session.h"

namespace smtk
{
namespace bridge
{
namespace openfoam
{

/// Return a shared pointer to the session backing an openFOAM operator.
SessionPtr Operator::activeSession()
{
  return smtk::dynamic_pointer_cast<smtk::bridge::openfoam::Session>(this->session());
}

} // namespace openfoam
} //namespace bridge
} // namespace smtk
