//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_AddImage_h
#define __smtk_model_AddImage_h

#include "smtk/model/operators/AddAuxiliaryGeometry.h"

namespace smtk
{
namespace model
{

class SMTKCORE_EXPORT AddImage : public AddAuxiliaryGeometry
{
public:
  smtkTypeMacro(smtk::model::AddImage);
  smtkCreateMacro(AddImage);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(AddAuxiliaryGeometry);

private:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} //namespace model
} // namespace smtk

#endif // __smtk_model_AddImage_h
