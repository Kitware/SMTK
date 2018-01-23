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

#include "smtk/operation/NewOp.h"

namespace smtk
{
namespace operation
{

/**\brief A class for adding python operators to the current session.

   Given a python file that describes an operator, this operator loads the
   python operator into the current session. The new operator is ready for use
   upon the successful completion of this operation (the session does not need
   to be restarted).
  */
class SMTKCORE_EXPORT ImportPythonOperator : public NewOp
{
public:
  smtkTypeMacro(ImportPythonOperator);
  smtkCreateMacro(ImportPythonOperator);
  smtkSharedFromThisMacro(smtk::operation::NewOp);
  smtkSuperclassMacro(smtk::operation::NewOp);

  virtual bool ableToOperate() override;

protected:
  Result operateInternal() override;
  Specification createSpecification() override;
};
}
}

#endif // __smtk_model_ImportPythonOperator_h
