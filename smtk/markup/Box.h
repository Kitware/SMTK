//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Box_h
#define smtk_markup_Box_h

#include "smtk/markup/AnalyticShape.h"

namespace smtk
{
namespace markup
{

/// A 3-dimensional, 6-sided shape with axis-aligned planar surfaces.
class SMTKMARKUP_EXPORT Box : public smtk::markup::AnalyticShape
{
public:
  smtkTypeMacro(smtk::markup::Box);
  smtkSuperclassMacro(smtk::markup::AnalyticShape);

  template<typename... Args>
  Box(Args&&... args)
    : smtk::markup::AnalyticShape(std::forward<Args>(args)...)
  {
  }

  ~Box() override = default;

  /// Provide initializers for resources to call after construction.
  /// These are called by the resource's createNode() method if extra parameters are passed.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;
  void initialize(const std::array<double, 3>& lo, const std::array<double, 3>& hi);

  bool setRange(const std::array<std::array<double, 3>, 2>& range);
  const std::array<std::array<double, 3>, 2>& range() const;
  std::array<std::array<double, 3>, 2>& range();

  /// Assign this node's state from \a source.
  bool assign(const smtk::graph::Component::ConstPtr& source, smtk::resource::CopyOptions& options)
    override;

protected:
  std::array<std::array<double, 3>, 2> m_range;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Box_h
