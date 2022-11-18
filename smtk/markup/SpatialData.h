//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_SpatialData_h
#define smtk_markup_SpatialData_h

#include "smtk/markup/Component.h"

namespace smtk
{
namespace markup
{

/**\brief Markup nodes that have spatial extents.
  *
  * Spatial data has a map from some abstract space (a Domain)
  * into some physical coordinate system that can be rendered
  * and analysed.
  *
  * Subclasses of SpatialData include DiscreteGeometry (where
  * the domain is represented as a complex of discrete primitive
  * shapes whose geometry is a (generally convex) combination of
  * corner points with a prescribed topology) and analytic shapes
  * (where the domain is a set of functions that map parameter
  * values into physical coordinates).
  */
class SMTKMARKUP_EXPORT SpatialData : public smtk::markup::Component
{
public:
  smtkTypeMacro(smtk::markup::SpatialData);
  smtkSuperclassMacro(smtk::markup::Component);

  template<typename... Args>
  SpatialData(Args&&... args)
    : smtk::markup::Component(std::forward<Args>(args)...)
  {
  }

  ~SpatialData() override;

  /// Provide an initializer for resources to call after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

protected:
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_SpatialData_h
