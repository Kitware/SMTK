//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operators_LoadSMTKModel_h
#define smtk_model_operators_LoadSMTKModel_h

#include "smtk/operation/XMLOperator.h"

namespace smtk
{
namespace model
{

/**\brief Load an SMTK resource from a file.
  */
class SMTKCORE_EXPORT LoadSMTKModel : public smtk::operation::XMLOperator
{
public:
  smtkTypeMacro(LoadSMTKModel);
  smtkSharedPtrCreateMacro(smtk::operation::NewOp);
  smtkSuperclassMacro(smtk::operation::XMLOperator);

protected:
  LoadSMTKModel();

  Result operateInternal() override;

  virtual const char* xmlDescription() const override;
  void generateSummary(Result&) override;
};

} //namespace model
} // namespace smtk

#endif
