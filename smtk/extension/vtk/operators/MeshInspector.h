//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_geometry_MeshInspector_h
#define smtk_geometry_MeshInspector_h

#include "smtk/extension/vtk/operators/vtkSMTKOperationsExtModule.h"
#include "smtk/geometry/Resource.h"
#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace geometry
{

/**\brief Inspect a volumetric mesh with a crinkle-slice.
 *
 * This operation does no work; it is used for inspection only
 * (and perhaps creating primitive selections at a later date).
 */
class VTKSMTKOPERATIONSEXT_EXPORT MeshInspector : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::geometry::MeshInspector);
  smtkCreateMacro(MeshInspector);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
  void generateSummary(Result&) override {}
};

} // namespace geometry
} // namespace smtk

#endif //smtk_geometry_MeshInspector_h
