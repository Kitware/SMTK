//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operators_GroupAuxiliaryGeometry_h
#define smtk_model_operators_GroupAuxiliaryGeometry_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace model
{

class SMTKCORE_EXPORT GroupAuxiliaryGeometry : public Operator
{
public:
  smtkTypeMacro(GroupAuxiliaryGeometry);
  smtkCreateMacro(GroupAuxiliaryGeometry);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

} //namespace model
} // namespace smtk

#endif // smtk_model_operators_GroupAuxiliaryGeometry_h
