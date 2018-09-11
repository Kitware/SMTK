//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_SetProperty_h
#define __smtk_model_SetProperty_h

#include "smtk/session/discrete/Operation.h"

namespace smtk
{
namespace session
{
namespace discrete
{

class SMTKDISCRETESESSION_EXPORT SetProperty : public Operation
{
public:
  smtkTypeMacro(smtk::session::discrete::SetProperty);
  smtkCreateMacro(SetProperty);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;

  template <typename V, typename VL, typename VD, typename VI>
  void setPropertyValue(
    const std::string& name, typename VI::Ptr item, smtk::model::EntityRefArray& entities);

  void setName(const std::string& name, smtk::model::EntityRefArray& entities);
  void setColor(smtk::attribute::DoubleItemPtr color, smtk::model::EntityRefArray& entities);
  void setVisibility(int visibility, smtk::model::EntityRefArray& entities);
};

} //namespace discrete
} //namespace session
} // namespace smtk

#endif // __smtk_model_SetProperty_h
