//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_geometry_DataSetInfoInspector_h
#define smtk_geometry_DataSetInfoInspector_h

#include "smtk/extension/vtk/operators/vtkSMTKOperationsExtModule.h"
#include "smtk/geometry/Resource.h"
#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace geometry
{

/**\brief Inspect mesh statistics on components with renderable geometry.
  *
  */
class VTKSMTKOPERATIONSEXT_EXPORT DataSetInfoInspector : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::geometry::DataSetInfoInspector);
  smtkCreateMacro(DataSetInfoInspector);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
  void generateSummary(Result&) override {}
};

} // namespace geometry
} // namespace smtk

#endif //smtk_geometry_DataSetInfoInspector_h
