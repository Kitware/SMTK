//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_cgm_CreateCylinder_h
#define __smtk_session_cgm_CreateCylinder_h

#include "smtk/bridge/cgm/Operator.h"

namespace smtk
{
namespace bridge
{
namespace cgm
{

/**\brief Create a cylinder given height, major and minor radii, and a number of sides.
  *
  * The number of sides must be 3 or greater.
  */
class SMTKCGMSESSION_EXPORT CreateCylinder : public Operator
{
public:
  smtkTypeMacro(CreateCylinder);
  smtkCreateMacro(CreateCylinder);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

} // namespace cgm
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_cgm_CreateCylinder_h
