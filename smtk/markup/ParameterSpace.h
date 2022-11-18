//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_ParameterSpace_h
#define smtk_markup_ParameterSpace_h

#include "smtk/markup/Domain.h"

namespace smtk
{
namespace markup
{
class DiscreteGeometry;
}
} // namespace smtk

namespace smtk
{
namespace markup
{

/// A dataset whose spatial extents serve as the domain of a map into a different coordinate system.
class SMTKMARKUP_EXPORT ParameterSpace : public smtk::markup::Domain
{
public:
  smtkTypeMacro(smtk::markup::ParameterSpace);
  smtkSuperclassMacro(smtk::markup::Domain);

  ParameterSpace() = default;
  ParameterSpace(smtk::string::Token name);
  ParameterSpace(const nlohmann::json& data);
  ~ParameterSpace() override = default;

  bool setData(const std::weak_ptr<smtk::markup::DiscreteGeometry>& data);
  const std::weak_ptr<smtk::markup::DiscreteGeometry>& data() const;
  std::weak_ptr<smtk::markup::DiscreteGeometry>& data();

protected:
  std::weak_ptr<smtk::markup::DiscreteGeometry> m_data;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_ParameterSpace_h
