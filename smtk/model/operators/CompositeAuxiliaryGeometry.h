//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operators_CompositeAuxiliaryGeometry_h
#define smtk_model_operators_CompositeAuxiliaryGeometry_h

#include "smtk/model/operators/AddAuxiliaryGeometry.h"

namespace smtk
{
namespace model
{

class SMTKCORE_EXPORT CompositeAuxiliaryGeometry : public AddAuxiliaryGeometry
{
public:
  smtkTypeMacro(smtk::model::CompositeAuxiliaryGeometry);
  smtkCreateMacro(CompositeAuxiliaryGeometry);
  smtkSuperclassMacro(AddAuxiliaryGeometry);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  const char* xmlDescription() const override;
};

} //namespace model
} // namespace smtk

#endif // smtk_model_operators_CompositeAuxiliaryGeometry_h
