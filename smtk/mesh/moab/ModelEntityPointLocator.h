//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_mesh_moab_ModelEntityPointLocator_h
#define smtk_mesh_moab_ModelEntityPointLocator_h

#include "smtk/model/PointLocatorExtension.h"

namespace smtk
{
namespace mesh
{
namespace moab
{

/**\brief A class that provides point-location based on entities' associated meshsets.
  */
class SMTKCORE_EXPORT ModelEntityPointLocator : public smtk::model::PointLocatorExtension
{
public:
  smtkTypeMacro(smtk::mesh::moab::ModelEntityPointLocator);
  smtkCreateMacro(smtk::common::Extension);
  smtkSuperclassMacro(smtk::model::PointLocatorExtension);
  virtual ~ModelEntityPointLocator();

  /// Overwrites \a closestPoints with points on \a entity closest to \a sourcePoints.
  bool closestPointOn(const smtk::model::EntityRef& entity, std::vector<double>& closestPoints,
    const std::vector<double>& sourcePoints, bool snapToPoint) override;

protected:
  ModelEntityPointLocator();
};
}
}
}

#endif
