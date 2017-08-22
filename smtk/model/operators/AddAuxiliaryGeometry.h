//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operators_AddAuxiliaryGeometry_h
#define smtk_model_operators_AddAuxiliaryGeometry_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace model
{

class SMTKCORE_EXPORT AddAuxiliaryGeometry : public Operator
{
public:
  smtkTypeMacro(AddAuxiliaryGeometry);
  smtkCreateMacro(AddAuxiliaryGeometry);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  smtk::model::OperatorResult operateInternal() override;
};

} //namespace model
} // namespace smtk

#endif // smtk_model_operators_AddAuxiliaryGeometry_h
