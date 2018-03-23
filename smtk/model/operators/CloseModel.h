//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_CloseModel_h
#define __smtk_model_CloseModel_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace model
{

class SMTKCORE_EXPORT CloseModel : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::model::CloseModel);
  smtkCreateMacro(CloseModel);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} //namespace model
} // namespace smtk

#endif // __smtk_model_CloseModel_h
