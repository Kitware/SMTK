//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_ExportEdgesToVTK_h
#define smtk_model_ExportEdgesToVTK_h
#ifndef __VTK_WRAP__

#include "smtk/extension/vtk/operators/vtkSMTKOperationsExtModule.h" // For export macro
#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace model
{

class VTKSMTKOPERATIONSEXT_EXPORT ExportEdgesToVTK : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(ExportEdgesToVTK);
  smtkCreateMacro(ExportEdgesToVTK);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} //namespace model
} // namespace smtk

#endif
#endif // smtk_model_ExportEdgesToVTK_h
