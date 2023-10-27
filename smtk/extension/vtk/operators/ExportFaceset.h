//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_geometry_ExportFaceset_h
#define smtk_geometry_ExportFaceset_h

#include "smtk/extension/vtk/operators/vtkSMTKOperationsExtModule.h"
#include "smtk/geometry/Resource.h"
#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace geometry
{

/**\brief Export a selected FaceSet to STL or other VTK surface formats.
 *
 * This operation does no work; it is used for inspection only
 * (and perhaps creating primitive selections at a later date).
 */
class VTKSMTKOPERATIONSEXT_EXPORT ExportFaceset : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::geometry::ExportFaceset);
  smtkCreateMacro(ExportFaceset);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace geometry
} // namespace smtk

#endif //smtk_geometry_ExportFaceset_h
