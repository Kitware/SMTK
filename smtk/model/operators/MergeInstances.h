//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operators_MergeInstances_h
#define smtk_model_operators_MergeInstances_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace model
{

class SMTKCORE_EXPORT MergeInstances : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::model::MergeInstances);
  smtkCreateMacro(MergeInstances);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace model
} // namespace smtk

#endif // smtk_model_operators_MergeInstances_h
