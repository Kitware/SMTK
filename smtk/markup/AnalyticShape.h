//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_AnalyticShape_h
#define smtk_markup_AnalyticShape_h

#include "smtk/markup/SpatialData.h"

namespace smtk
{
namespace markup
{

/**\brief Simple shapes that have analytic representations, generally as implicit,
  *       trivariate functions whose zero level-set defines the shape.
  */
class SMTKMARKUP_EXPORT AnalyticShape : public smtk::markup::SpatialData
{
public:
  smtkTypeMacro(smtk::markup::AnalyticShape);
  smtkSuperclassMacro(smtk::markup::SpatialData);

  template<typename... Args>
  AnalyticShape(Args&&... args)
    : smtk::markup::SpatialData(std::forward<Args>(args)...)
  {
  }

  ~AnalyticShape() override;

  /// Provide an initializer for resources to call after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

protected:
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_AnalyticShape_h
