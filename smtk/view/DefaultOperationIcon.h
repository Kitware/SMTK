//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_DefaultOperationIcon_h
#define smtk_view_DefaultOperationIcon_h

#include "smtk/common/Color.h"

#include <regex>
#include <string>

#include "smtk/view/icons/default_operation_opt_svg.h"

namespace smtk
{
namespace view
{

std::string SMTKCORE_EXPORT DefaultOperationIcon(const std::string& secondaryColor)
{
  std::string svg = default_operation_opt_svg;
  std::array<double, 4> rgba;
  if (smtk::common::Color::stringToFloatRGBA(rgba.data(), secondaryColor) &&
    smtk::common::Color::floatRGBToLightness(rgba.data()) > 0.5)
  {
    svg = std::regex_replace(svg, std::regex("stroke=\"#000"), "stroke=\"#fff");
  }
  return svg;
}
}
}

#endif
