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

#include "smtk/session/rgg/Exports.h"

#include <string>

namespace smtk
{
namespace session
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
  ~Material();

  operator std::string() const;

  const std::string& name() const;
  const double& density() const;
  const std::string& densityType() const;
  const double& temperature() const;
  const double& thermalExpansion() const;
  const std::string& compositionType() const;
  std::size_t numberOfComponents() const;
  const std::string& component(std::size_t i) const;
  const double& content(std::size_t i) const;

  void setName(const std::string& name);
  void setDensity(const double& density);
  void setDensityType(const std::string& densityType);
  void setTemperature(const double& temperature);
  void setThermalExpansion(const double& thermExpansion);
  void setCompositionType(const std::string& compType);
  void addComponent(const std::string& comp);
  void addContent(const double& content);

private:
  struct Internal;
  Internal* m_internal;
};

} // namespace rgg
} // namespace session
} // namespace smtk

#endif // __smtk_session_rgg_Material_h
