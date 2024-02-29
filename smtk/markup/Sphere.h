//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Sphere_h
#define smtk_markup_Sphere_h

#include "smtk/markup/AnalyticShape.h"

namespace smtk
{
namespace markup
{

/// A 3-dimensional surface of constant non-zero curvature identified by a point and radius.
class SMTKMARKUP_EXPORT Sphere : public smtk::markup::AnalyticShape
{
public:
  smtkTypeMacro(smtk::markup::Sphere);
  smtkSuperclassMacro(smtk::markup::AnalyticShape);

  template<typename... Args>
  Sphere(Args&&... args)
    : smtk::markup::AnalyticShape(std::forward<Args>(args)...)
  {
  }

  ~Sphere() override = default;

  /// Provide initializers for resources to call after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;
  void initialize(const std::array<double, 3>& center, double radius);
  void initialize(const std::array<double, 3>& center, const std::array<double, 3>& radii);

  bool setCenter(const std::array<double, 3>& center);
  const std::array<double, 3>& center() const;
  std::array<double, 3>& center();

  bool setRadius(const std::array<double, 3>& radius);
  const std::array<double, 3>& radius() const;
  std::array<double, 3>& radius();

  /// Assign this node's state from \a source.
  bool assign(const smtk::graph::Component::ConstPtr& source, smtk::resource::CopyOptions& options)
    override;

protected:
  std::array<double, 3> m_center;
  std::array<double, 3> m_radius;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Sphere_h
