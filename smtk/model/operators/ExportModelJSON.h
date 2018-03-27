//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_ExportModelJSON_h
#define __smtk_model_ExportModelJSON_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace model
{

class SMTKCORE_EXPORT ExportModelJSON : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::model::ExportModelJSON);
  smtkCreateMacro(ExportModelJSON);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} //namespace model
} // namespace smtk

#endif // __smtk_model_ExportModelJSON_h
