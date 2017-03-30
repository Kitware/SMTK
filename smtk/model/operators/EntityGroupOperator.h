//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_EntityGroupOperator_h
#define __smtk_model_EntityGroupOperator_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace model
{

/**\brief Create, desctory or modify a model entity group.
  *
  * There are three operations available from this Operator class.
  * 1. Create, which will create a entity group with given "BuildEnityType".
  * 2. Destroy, which will remove a entity group with given entity Id;
  * 3. Modify/Operate, which will add or remove entities from the given group.
  *
  * If turn on advanced model, you would have control of what to add.
  */
class SMTKCORE_EXPORT EntityGroupOperator : public Operator
{
public:
  smtkTypeMacro(EntityGroupOperator);
  smtkCreateMacro(EntityGroupOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

} //namespace model
} // namespace smtk

#endif // __smtk_model_EntityGroup_h
