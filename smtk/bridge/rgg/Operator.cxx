//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/rgg/Operator.h"
#include "smtk/bridge/rgg/Session.h"

namespace smtk
{
namespace bridge
{
namespace rgg
{

/// Return a shared pointer to the session backing a Mesh operator.
SessionPtr Operator::activeSession()
{
  return smtk::dynamic_pointer_cast<smtk::bridge::rgg::Session>(this->session());
}

} // namespace rgg
} //namespace bridge
} // namespace smtk
