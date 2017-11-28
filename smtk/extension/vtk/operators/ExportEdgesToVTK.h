//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_ExportEdgesToVTK_h
#define __smtk_model_ExportEdgesToVTK_h
#ifndef __VTK_WRAP__

#include "smtk/extension/vtk/operators/Exports.h" // For export macro
#include "smtk/operation/XMLOperator.h"

namespace smtk
{
namespace model
{

class VTKSMTKOPERATORSEXT_EXPORT ExportEdgesToVTK : public smtk::operation::XMLOperator
{
public:
  smtkTypeMacro(ExportEdgesToVTK);
  smtkCreateMacro(ExportEdgesToVTK);
  smtkSharedFromThisMacro(smtk::operation::NewOp);
  smtkSuperclassMacro(smtk::operation::XMLOperator);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} //namespace model
} // namespace smtk

#endif
#endif // __smtk_model_ExportEdgesToVTK_h
