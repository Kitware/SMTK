//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Plane_h
#define smtk_markup_Plane_h

#include "smtk/markup/AnalyticShape.h"

namespace smtk
{
namespace markup
{

/// A flat 3-dimensional surface identified by a point and normal.
class SMTKMARKUP_EXPORT Plane : public smtk::markup::AnalyticShape
{
public:
  smtkTypeMacro(smtk::markup::Plane);
  smtkSuperclassMacro(smtk::markup::AnalyticShape);

  template<typename... Args>
  Plane(Args&&... args)
    : smtk::markup::AnalyticShape(std::forward<Args>(args)...)
  {
  }

  ~Plane() override;

  /// Provide an initializer for resources to call after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

  bool setBasePoint(const std::array<double, 3>& basePoint);
  const std::array<double, 3>& basePoint() const;
  std::array<double, 3>& basePoint();

  bool setNormal(const std::array<double, 3>& normal);
  const std::array<double, 3>& normal() const;
  std::array<double, 3>& normal();

protected:
  std::array<double, 3> m_basePoint;
  std::array<double, 3> m_normal;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Plane_h
