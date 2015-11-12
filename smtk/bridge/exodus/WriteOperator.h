//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_exodus_WriteOperator_h
#define __smtk_session_exodus_WriteOperator_h

#include "smtk/bridge/exodus/Operator.h"

namespace smtk {
  namespace bridge {
    namespace exodus {

class SMTKEXODUSSESSION_EXPORT WriteOperator : public Operator
{
public:
  smtkTypeMacro(WriteOperator);
  smtkCreateMacro(WriteOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();
  virtual smtk::model::OperatorResult writeExodus();
  virtual smtk::model::OperatorResult writeSLAC();
  virtual smtk::model::OperatorResult writeLabelMap();
};

    } // namespace exodus
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_exodus_WriteOperator_h
