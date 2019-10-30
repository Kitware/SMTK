//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_smtkModelEntityPointLocator_h
#define smtk_extension_paraview_server_smtkModelEntityPointLocator_h

#include "smtk/extension/paraview/server/Exports.h"
#include "smtk/model/PointLocatorExtension.h"

/**\brief A class that provides point-location based on the VTK tessellation of entities.
  */
class SMTKPVSERVEREXT_EXPORT smtkModelEntityPointLocator : public smtk::model::PointLocatorExtension
{
public:
  smtkTypeMacro(smtkModelEntityPointLocator);
  smtkCreateMacro(smtk::common::Extension);
  smtkSuperclassMacro(smtk::model::PointLocatorExtension);
  virtual ~smtkModelEntityPointLocator();

  /// Overwrites \a closestPoints with points on \a entity closest to \a sourcePoints.
  bool closestPointOn(const smtk::model::EntityRef& entity, std::vector<double>& closestPoints,
    const std::vector<double>& sourcePoints, bool snapToPoint) override;

  bool randomPoint(const smtk::model::EntityRef& entity, const std::size_t nPoints,
    std::vector<double>& points, const std::size_t seed) override;

protected:
  smtkModelEntityPointLocator();
};

#endif
