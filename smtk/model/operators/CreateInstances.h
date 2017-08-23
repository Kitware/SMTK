//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operators_CreateInstances_h
#define smtk_model_operators_CreateInstances_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace model
{

class SMTKCORE_EXPORT CreateInstances : public Operator
{
public:
  smtkTypeMacro(CreateInstances);
  smtkCreateMacro(CreateInstances);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  smtk::model::OperatorResult operateInternal() override;

  void addTabularRule(Instance& instance, const EntityRef& prototype);
  void addUniformRandomRule(Instance& instance, const EntityRef& prototype);
  void addSnappingConstraints(Instance& instance, const EntityRef& prototype);
};

} //namespace model
} // namespace smtk

#endif // __smtk_model_operators_CreateInstances_h
