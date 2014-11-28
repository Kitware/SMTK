//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_bridge_cgm_CreatePrism_h
#define __smtk_bridge_cgm_CreatePrism_h

#include "smtk/bridge/cgm/Operator.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

/**\brief Create a prism given height, major and minor radii, and a number of sides.
  *
  * The number of sides must be 3 or greater.
  */
class CGMSMTK_EXPORT CreatePrism : public Operator
{
public:
  smtkTypeMacro(CreatePrism);
  smtkCreateMacro(CreatePrism);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

#endif // __smtk_bridge_cgm_CreatePrism_h
