//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_exodus_ReadOperator_h
#define __smtk_session_exodus_ReadOperator_h

#include "smtk/bridge/exodus/Operator.h"

namespace smtk {
  namespace bridge {
    namespace exodus {

class SMTKEXODUSSESSION_EXPORT ReadOperator : public Operator
{
public:
  smtkTypeMacro(ReadOperator);
  smtkCreateMacro(ReadOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

    } // namespace exodus
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_exodus_ReadOperator_h
