//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_cgm_BooleanIntersection_h
#define __smtk_session_cgm_BooleanIntersection_h

#include "smtk/bridge/cgm/Operator.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

class SMTKCGMSESSION_EXPORT BooleanIntersection : public Operator
{
public:
  smtkTypeMacro(BooleanIntersection);
  smtkCreateMacro(BooleanIntersection);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

#endif // __smtk_session_cgm_BooleanIntersection_h
