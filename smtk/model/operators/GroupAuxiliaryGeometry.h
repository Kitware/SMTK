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

#include "smtk/operation/XMLOperator.h"

namespace smtk
{
namespace model
{

class SMTKCORE_EXPORT GroupAuxiliaryGeometry : public smtk::operation::XMLOperator
{
public:
  smtkTypeMacro(GroupAuxiliaryGeometry);
  smtkCreateMacro(GroupAuxiliaryGeometry);
  smtkSharedFromThisMacro(smtk::operation::NewOp);
  smtkSuperclassMacro(smtk::operation::XMLOperator);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} //namespace model
} // namespace smtk

#endif // smtk_model_operators_GroupAuxiliaryGeometry_h
