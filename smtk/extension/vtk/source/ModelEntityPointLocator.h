//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_vtk_source_ModelEntityPointLocator_h
#define smtk_extension_vtk_source_ModelEntityPointLocator_h

#include "smtk/extension/vtk/source/vtkSMTKSourceExtModule.h"
#include "smtk/model/PointLocatorExtension.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace source
{

/**\brief A class that provides point-location based on the VTK tessellation of entities.
  */
class VTKSMTKSOURCEEXT_EXPORT ModelEntityPointLocator : public smtk::model::PointLocatorExtension
{
public:
  smtkTypeMacro(smtk::extension::vtk::source::ModelEntityPointLocator);
  smtkCreateMacro(smtk::common::Extension);
  smtkSuperclassMacro(smtk::model::PointLocatorExtension);
  ~ModelEntityPointLocator() override;

  /// Overwrites \a closestPoints with points on \a entity closest to \a sourcePoints.
  bool closestPointOn(
    const smtk::model::EntityRef& entity,
    std::vector<double>& closestPoints,
    const std::vector<double>& sourcePoints,
    bool snapToPoint) override;

  bool randomPoint(
    const smtk::model::EntityRef& entity,
    const std::size_t nPoints,
    std::vector<double>& points,
    const std::size_t seed) override;

protected:
  ModelEntityPointLocator();
};
} // namespace source
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif
