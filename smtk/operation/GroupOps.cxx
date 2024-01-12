//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/GroupOps.h"

namespace smtk
{
namespace operation
{

bool DispositionBatch::ableToOperate() const
{
  return !ops.empty() &&              // Must have at least one op to launch
    numberOfBlockedOperations == 0 && // No ops may be blocked.
    noMatch.empty() &&                // All inputs must match an operation in the group.
    cannotAssociate.empty();          // All inputs must be associated to an op.
}

} // namespace operation
} // namespace smtk
