//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/AnalyticShape.h"

namespace smtk
{
namespace markup
{

AnalyticShape::~AnalyticShape() = default;

void AnalyticShape::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  (void)data;
  (void)helper;
}

} // namespace markup
} // namespace smtk
