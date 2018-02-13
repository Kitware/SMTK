//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operators_SaveSMTKModel_h
#define smtk_model_operators_SaveSMTKModel_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace model
{

/// An operator that uses resource metadata to write resources.
class SMTKCORE_EXPORT SaveSMTKModel : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(SaveSMTKModel);
  smtkSharedPtrCreateMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  SaveSMTKModel();

  Result operateInternal() override;

  virtual const char* xmlDescription() const override;
  void generateSummary(Result&) override;
};

} //namespace model
} // namespace smtk

#endif
