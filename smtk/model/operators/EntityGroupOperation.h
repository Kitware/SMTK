//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_EntityGroupOperation_h
#define smtk_model_EntityGroupOperation_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace model
{

/**\brief Create, desctory or modify a model entity group.
  *
  * There are three operations available from this Operation class.
  * 1. Create, which will create a entity group with given "BuildEnityType".
  * 2. Destroy, which will remove a entity group with given entity Id;
  * 3. Modify/Operate, which will add or remove entities from the given group.
  *
  * If turn on advanced model, you would have control of what to add.
  */
class SMTKCORE_EXPORT EntityGroupOperation : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::model::EntityGroupOperation);
  smtkCreateMacro(EntityGroupOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} //namespace model
} // namespace smtk

#endif // smtk_model_EntityGroup_h
