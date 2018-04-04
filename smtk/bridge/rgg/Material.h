//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_rgg_Material_h
#define __smtk_session_rgg_Material_h

#include "smtk/bridge/rgg/Exports.h"

#include <string>
#include <vector>

namespace smtk
{
namespace bridge
{
namespace rgg
{

/**\brief A material description for RGG"
  */
struct SMTKRGGSESSION_EXPORT Material
{
  static constexpr const char* const label = "material_descriptions";

  Material();
  Material(const std::string&);

  operator std::string() const;

  std::string m_name;
  double m_density;
  std::string m_densityType;
  double m_temperature;
  double m_thermalExpansion;
  std::string m_compositionType;
  std::vector<std::string> m_components;
  std::vector<double> m_content;
};

} // namespace rgg
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_rgg_Material_h
