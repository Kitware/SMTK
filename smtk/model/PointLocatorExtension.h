//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_PointLocatorExtension_h
#define smtk_model_PointLocatorExtension_h

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
  ~PointLocatorExtension() override;

  /// Overwrites \a closestPoints with points on \a entity closest to
  /// \a sourcePoints. If \a snapToPoint, the nearest point explicitly defined
  /// in the entity is returned. Otherwise, the nearest point on the entity's
  /// surface is returned.
  virtual bool closestPointOn(
    const EntityRef& entity,
    std::vector<double>& closestPoints,
    const std::vector<double>& sourcePoints,
    bool snapToPoint) = 0;

  /// Generate \a nPoints random points on \a entity and store the results in
  /// \a points. Return true if successful.
  virtual bool randomPoint(
    const EntityRef& entity,
    std::size_t nPoints,
    std::vector<double>& points,
    std::size_t seed) = 0;

  /// Same as above, but seeded with a hardware-supplied random integer.
  bool randomPoint(const EntityRef& entity, std::size_t nPoints, std::vector<double>& points);

protected:
  PointLocatorExtension();
};
} // namespace model
} // namespace smtk

#endif
