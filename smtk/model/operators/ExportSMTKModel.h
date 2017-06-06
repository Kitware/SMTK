//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_ExportSMTKModel_h
#define __smtk_model_ExportSMTKModel_h

#include "smtk/model/operators/SaveSMTKModel.h"

namespace smtk
{
namespace model
{

/// Export an SMTK model (identical server-side operation to saving; we override nothing).
class SMTKCORE_EXPORT ExportSMTKModel : public SaveSMTKModel
{
public:
  smtkTypeMacro(ExportSMTKModel);
  smtkCreateMacro(ExportSMTKModel);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  ExportSMTKModel();
};

} //namespace model
} // namespace smtk

#endif // __smtk_model_ExportSMTKModel_h
