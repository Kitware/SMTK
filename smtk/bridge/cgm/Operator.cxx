//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/Operator.h"
#include "smtk/bridge/cgm/Bridge.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

/// Return a shared pointer to the bridge backing a CGM operator.
Bridge* Operator::cgmBridge()
{
  return dynamic_cast<smtk::bridge::cgm::Bridge*>(this->bridge());
}

} // namespace cgm
  } //namespace bridge
} // namespace smtk
