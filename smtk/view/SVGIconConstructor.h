//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_SVGIconConstructor_h
#define smtk_view_SVGIconConstructor_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include <string>

namespace smtk
{
namespace resource
{
class PersistentObject;
}
namespace view
{
class SMTKCORE_EXPORT SVGIconConstructor
{
public:
  SVGIconConstructor(const std::string& defaultColor, const std::string& secondaryColor)
    : m_defaultColor(defaultColor)
    , m_secondaryColor(secondaryColor)
  {
  }

  SVGIconConstructor()
    : m_defaultColor("green")
    , m_secondaryColor("black")
  {
  }

  std::string operator()(const smtk::resource::PersistentObject&, const std::string&) const;

protected:
  virtual std::string svg(const smtk::resource::PersistentObject&) const = 0;

  std::string m_defaultColor;
  std::string m_secondaryColor;
};

class SMTKCORE_EXPORT DefaultIconConstructor
{
public:
  std::string operator()(const smtk::resource::PersistentObject&, const std::string&) const;
};

class SMTKCORE_EXPORT ResourceIconConstructor : public SVGIconConstructor
{
  std::string svg(const smtk::resource::PersistentObject&) const override;
};

class SMTKCORE_EXPORT AttributeIconConstructor : public SVGIconConstructor
{
  std::string svg(const smtk::resource::PersistentObject&) const override;
};

class SMTKCORE_EXPORT ModelIconConstructor : public SVGIconConstructor
{
  std::string svg(const smtk::resource::PersistentObject&) const override;
};
} // namespace view
} // namespace smtk

#endif
