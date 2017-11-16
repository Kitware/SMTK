//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_ImportPythonOperator_h
#define __smtk_model_ImportPythonOperator_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace model
{

/**\brief A class for adding python operators to the current session.

   Given a python file that describes an operator, this operator loads the
   python operator into the current session. The new operator is ready for use
   upon the successful completion of this operation (the session does not need
   to be restarted).
  */
class SMTKCORE_EXPORT ImportPythonOperator : public Operator
{
public:
  smtkTypeMacro(ImportPythonOperator);
  smtkCreateMacro(ImportPythonOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  smtk::model::OperatorResult operateInternal() override;
};
}
}

#endif // __smtk_model_ImportPythonOperator_h
