//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtUtility.h"

#include "smtk/extension/qt/SVGIconEngine.h"

#include "smtk/Regex.h"

namespace smtk
{
namespace extension
{

QIcon colorAdjustedIcon(const std::string& svg, const QColor& background)
{
  std::string adjusted =
    background.lightnessF() >= 0.5 ? svg : smtk::regex_replace(svg, smtk::regex("black"), "white");
  return QIcon(new smtk::extension::SVGIconEngine(adjusted));
}

} // namespace extension
} // namespace smtk
