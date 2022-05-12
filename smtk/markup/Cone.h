//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Cone_h
#define smtk_markup_Cone_h

#include "smtk/markup/AnalyticShape.h"

namespace smtk
{
namespace markup
{

/// A ruled 3-dimensional surface identified by 2 points and 2 radii.
class SMTKMARKUP_EXPORT Cone : public smtk::markup::AnalyticShape
{
public:
  smtkTypeMacro(smtk::markup::Cone);
  smtkSuperclassMacro(smtk::markup::AnalyticShape);

  template<typename... Args>
  Cone(Args&&... args)
    : smtk::markup::AnalyticShape(std::forward<Args>(args)...)
  {
  }

  ~Cone() override;

  /// Provide an initializer for resources to call after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

  bool setEndpoints(const std::array<std::array<double, 3>, 2>& endpoints);
  const std::array<std::array<double, 3>, 2>& endpoints() const;
  std::array<std::array<double, 3>, 2>& endpoints();

  bool setRadii(const std::array<double, 2>& radii);
  const std::array<double, 2>& radii() const;
  std::array<double, 2>& radii();

protected:
  std::array<std::array<double, 3>, 2> m_endpoints;
  std::array<double, 2> m_radii;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Cone_h
