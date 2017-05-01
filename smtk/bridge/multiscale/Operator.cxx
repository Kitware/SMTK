//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/multiscale/Operator.h"
#include "smtk/bridge/multiscale/Session.h"

namespace smtk
{
namespace bridge
{
namespace multiscale
{

/// Return a shared pointer to the session backing a multiscale operator.
SessionPtr Operator::activeSession()
{
  return smtk::dynamic_pointer_cast<smtk::bridge::multiscale::Session>(this->session());
}

} // namespace multiscale
} //namespace bridge
} // namespace smtk
