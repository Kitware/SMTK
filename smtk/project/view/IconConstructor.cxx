//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/project/view/IconConstructor.h"

#include "smtk/Regex.h"

namespace smtk
{
namespace project
{
namespace view
{
std::string IconConstructor::operator()(const std::string& secondaryColor) const
{
  std::string fill = "gray";

  std::string svg = smtk::regex_replace(
    smtk::regex_replace(this->svg(), smtk::regex(m_defaultColor), fill),
    smtk::regex(m_secondaryColor),
    secondaryColor);

  return svg;
}
} // namespace view
} // namespace project
} // namespace smtk
