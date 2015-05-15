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

#include "smtk/bridge/discrete/Operator.h"

namespace smtk {
  namespace bridge {
    namespace discrete {

class SMTKDISCRETESESSION_EXPORT SetProperty : public smtk::bridge::discrete::Operator
{
public:
  smtkTypeMacro(SetProperty);
  smtkCreateMacro(SetProperty);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();

  template<typename V, typename VL, typename VD, typename VI>
  void setPropertyValue(
    const std::string& name,
    typename VI::Ptr item,
    smtk::model::EntityRefArray& entities);

  void setName(const std::string& name, smtk::model::EntityRefArray& entities);
  void setColor(smtk::attribute::DoubleItemPtr color, smtk::model::EntityRefArray& entities);
  void setVisibility(int visibility, smtk::model::EntityRefArray& entities);
};

    } //namespace discrete
  } //namespace bridge
} // namespace smtk

#endif // __smtk_model_SetProperty_h
