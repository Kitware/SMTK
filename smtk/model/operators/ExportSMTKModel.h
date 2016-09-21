//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_ExportSMTKModel_h
#define __smtk_model_ExportSMTKModel_h

#include "smtk/model/Operator.h"

namespace smtk {
  namespace model {

class SMTKCORE_EXPORT ExportSMTKModel : public Operator
{
public:
  smtkTypeMacro(ExportSMTKModel);
  smtkCreateMacro(ExportSMTKModel);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual OperatorResult operateInternal();
  virtual void generateSummary(OperatorResult&);
};

  } //namespace model
} // namespace smtk

#endif // __smtk_model_ExportSMTKModel_h
