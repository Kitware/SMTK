//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_multiscale_PartitionBoundaries_h
#define __smtk_session_multiscale_PartitionBoundaries_h

#include "smtk/bridge/multiscale/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace bridge
{
namespace multiscale
{

/**\brief An operator for partitioning the AFRL Phase I Demo model.
 *
 * For the AFRL Materials Phase I Demo, takes the 3-dimensional mesh and applies
 * cooling plate and ambient air Dirichlet boundaries.
*/

class SMTKMULTISCALESESSION_EXPORT PartitionBoundaries : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::bridge::multiscale::PartitionBoundaries);
  smtkCreateMacro(PartitionBoundaries);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace multiscale
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_multiscale_PartitionBoundaries_h
