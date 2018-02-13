//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_cgm_CreateEdge_h
#define __smtk_session_cgm_CreateEdge_h

#include "smtk/bridge/cgm/Operation.h"

namespace smtk
{
namespace bridge
{
namespace cgm
{

/**\brief Create a edge given height, major and minor radii, and a number of sides.
  *
  * The number of sides must be 3 or greater.
  */
class SMTKCGMSESSION_EXPORT CreateEdge : public Operation
{
public:
  smtkTypeMacro(CreateEdge);
  smtkCreateMacro(CreateEdge);
  smtkSharedFromThisMacro(Operation);
  smtkDeclareModelOperation();

protected:
  Result operateInternal() override;
};

} // namespace cgm
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_cgm_CreateEdge_h
