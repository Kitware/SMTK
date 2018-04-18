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
#include <vector>

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

struct Material::Internal
{
  std::string m_name;
  double m_density;
  std::string m_densityType;
  double m_temperature;
  double m_thermalExpansion;
  std::string m_compositionType;
  std::vector<std::string> m_components;
  std::vector<double> m_content;
};

Material::Material()
  : m_internal(new Internal)
{
}

Material::Material(const std::string& sonDescription)
  : m_internal(new Internal)
{
  m_internal->m_name = trim(substring(
    sonDescription, sonDescription.find_first_of("(") + 1, sonDescription.find_first_of(")")));
  m_internal->m_densityType =
    (sonDescription.find("adensity") == std::string::npos) ? "wdensity" : "adensity";
  std::size_t tmp = sonDescription.find(m_internal->m_densityType);
  m_internal->m_density = std::stod(
    substring(sonDescription, sonDescription.find("=", tmp) + 1, sonDescription.find("\n", tmp)));

  tmp = sonDescription.find("temp");
  m_internal->m_temperature = std::stod(
    substring(sonDescription, sonDescription.find("=", tmp) + 1, sonDescription.find("\n", tmp)));

  tmp = sonDescription.find("therm_exp");
  m_internal->m_thermalExpansion = std::stod(
    substring(sonDescription, sonDescription.find("=", tmp) + 1, sonDescription.find("\n", tmp)));

  tmp = sonDescription.find("{", sonDescription.find("{") + 1);
  m_internal->m_compositionType =
    trim(substring(sonDescription, sonDescription.rfind("\n", tmp) + 1, tmp));

  std::string comp =
    m_internal->m_compositionType.substr(0, m_internal->m_compositionType.size() - 1);

  std::size_t lineStart = sonDescription.find("\n", tmp) + 1;
  std::size_t lineEnd = sonDescription.find("\n", lineStart);
  std::size_t end = sonDescription.find("}", tmp);
  while (lineEnd < end)
  {
    std::string line = substring(sonDescription, lineStart, lineEnd);
    m_internal->m_components.push_back(
      trim(substring(line, line.find_first_of("(") + 1, line.find_first_of(")"))));
    m_internal->m_content.push_back(std::stod(substring(line, line.find("=") + 1)));
    lineStart = lineEnd + 1;
    lineEnd = sonDescription.find("\n", lineStart);
  }
}

Material::~Material()
{
  delete m_internal;
}

Material::operator std::string() const
{
  // Construct a SON description of the material (as defined in WASP).
  std::stringstream s;
  s << "material ( " << m_internal->m_name << " ) {\n";
  s << m_internal->m_densityType << " = " << m_internal->m_density << "\n";
  s << "temp = " << m_internal->m_temperature << "\n";
  s << "therm_exp = " << m_internal->m_thermalExpansion << "\n";
  s << m_internal->m_compositionType << "{\n";
  std::string comp =
    m_internal->m_compositionType.substr(0, m_internal->m_compositionType.size() - 1);
  for (std::size_t i = 0; i < m_internal->m_components.size(); i++)
  {
    s << comp << " ( " << m_internal->m_components[i] << " ) = " << m_internal->m_content[i]
      << "\n";
  }
  s << "}\n}";

  return s.str();
}

const std::string& Material::name() const
{
  return m_internal->m_name;
}

const double& Material::density() const
{
  return m_internal->m_density;
}

const std::string& Material::densityType() const
{
  return m_internal->m_densityType;
}

const double& Material::temperature() const
{
  return m_internal->m_temperature;
}

const double& Material::thermalExpansion() const
{
  return m_internal->m_thermalExpansion;
}

const std::string& Material::compositionType() const
{
  return m_internal->m_compositionType;
}

std::size_t Material::numberOfComponents() const
{
  return m_internal->m_components.size();
}

const std::string& Material::component(std::size_t i) const
{
  return m_internal->m_components.at(i);
}

const double& Material::content(std::size_t i) const
{
  return m_internal->m_content.at(i);
}

void Material::setName(const std::string& name)
{
  m_internal->m_name = name;
}

void Material::setDensity(const double& density)
{
  m_internal->m_density = density;
}

void Material::setDensityType(const std::string& densityType)
{
  m_internal->m_densityType = densityType;
}

void Material::setTemperature(const double& temperature)
{
  m_internal->m_temperature = temperature;
}

void Material::setThermalExpansion(const double& thermExpansion)
{
  m_internal->m_thermalExpansion = thermExpansion;
}

void Material::setCompositionType(const std::string& compType)
{
  m_internal->m_compositionType = compType;
}

void Material::addComponent(const std::string& comp)
{
  m_internal->m_components.push_back(comp);
}

void Material::addContent(const double& content)
{
  m_internal->m_content.push_back(content);
}

} // namespace rgg
} //namespace bridge
} // namespace smtk
