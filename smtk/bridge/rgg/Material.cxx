//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/rgg/Material.h"

#include <sstream>

namespace
{
std::string trim(const std::string& str)
{
  std::size_t first = str.find_first_not_of(' ');
  if (first == std::string::npos)
  {
    return str;
  }
  std::size_t last = str.find_last_not_of(' ');
  return str.substr(first, (last - first + 1));
}

std::string substring(const std::string& s, std::size_t pos1, std::size_t pos2 = std::string::npos)
{
  return s.substr(pos1, pos2 - pos1);
}
}

namespace smtk
{
namespace bridge
{
namespace rgg
{

Material::Material()
{
}

Material::Material(const std::string& sonDescription)
{
  m_name = trim(substring(
    sonDescription, sonDescription.find_first_of("(") + 1, sonDescription.find_first_of(")")));
  m_densityType = (sonDescription.find("adensity") == std::string::npos) ? "wdensity" : "adensity";
  std::size_t tmp = sonDescription.find(m_densityType);
  m_density = std::stod(
    substring(sonDescription, sonDescription.find("=", tmp) + 1, sonDescription.find("\n", tmp)));

  tmp = sonDescription.find("temp");
  m_temperature = std::stod(
    substring(sonDescription, sonDescription.find("=", tmp) + 1, sonDescription.find("\n", tmp)));

  tmp = sonDescription.find("therm_exp");
  m_thermalExpansion = std::stod(
    substring(sonDescription, sonDescription.find("=", tmp) + 1, sonDescription.find("\n", tmp)));

  tmp = sonDescription.find("{", sonDescription.find("{") + 1);
  m_compositionType = trim(substring(sonDescription, sonDescription.rfind("\n", tmp) + 1, tmp));

  std::string comp = m_compositionType.substr(0, m_compositionType.size() - 1);

  std::size_t lineStart = sonDescription.find("\n", tmp) + 1;
  ;
  std::size_t lineEnd = sonDescription.find("\n", lineStart);
  std::size_t end = sonDescription.find("}", tmp);
  while (lineEnd < end)
  {
    std::string line = substring(sonDescription, lineStart, lineEnd);
    m_components.push_back(
      trim(substring(line, line.find_first_of("(") + 1, line.find_first_of(")"))));
    m_content.push_back(std::stod(substring(line, line.find("=") + 1)));
    lineStart = lineEnd + 1;
    lineEnd = sonDescription.find("\n", lineStart);
  }
}

Material::operator std::string() const
{
  // Construct a SON description of the material (as defined in WASP).
  std::stringstream s;
  s << "material ( " << m_name << " ) {\n";
  s << m_densityType << " = " << m_density << "\n";
  s << "temp = " << m_temperature << "\n";
  s << "therm_exp = " << m_thermalExpansion << "\n";
  s << m_compositionType << "{\n";
  std::string comp = m_compositionType.substr(0, m_compositionType.size() - 1);
  for (std::size_t i = 0; i < m_components.size(); i++)
  {
    s << comp << " ( " << m_components[i] << " ) = " << m_content[i] << "\n";
  }
  s << "}\n}";

  return s.str();
}

} // namespace rgg
} //namespace bridge
} // namespace smtk
