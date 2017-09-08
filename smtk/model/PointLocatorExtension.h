//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#pragma once

#include "smtk/common/Extension.h"

#include <vector>

namespace smtk
{
namespace model
{

class EntityRef;

/**\brief A base class for extensions that provide point-locators
  * for finding nearest points on tessellations of model entities.
  */
class SMTKCORE_EXPORT PointLocatorExtension : public smtk::common::Extension
{
public:
  smtkTypeMacro(PointLocatorExtension);
  smtkSuperclassMacro(smtk::common::Extension);
  smtkSharedFromThisMacro(smtk::common::Extension);
  virtual ~PointLocatorExtension();

  /// Overwrites \a closestPoints with points on \a entity closest to \a sourcePoints.
  virtual bool closestPointOn(const EntityRef& entity, std::vector<double>& closestPoints,
    const std::vector<double>& sourcePoints) = 0;

protected:
  PointLocatorExtension();
};
}
}
