//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Feature_h
#define smtk_markup_Feature_h

#include "smtk/markup/Component.h"

namespace smtk
{
namespace markup
{

class SMTKMARKUP_EXPORT Feature : public smtk::markup::Component
{
public:
  smtkTypeMacro(smtk::markup::Feature);
  smtkSuperclassMacro(smtk::markup::Component);

  template<typename... Args>
  Feature(Args&&... args)
    : smtk::markup::Component(std::forward<Args>(args)...)
  {
  }

  ~Feature() override;

  /// Provide an initializer for resources to call after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

protected:
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Feature_h
