//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Index_h
#define smtk_markup_Index_h

#include "smtk/markup/Domain.h"

namespace smtk
{
namespace markup
{

/**\brief A catalog that orders numbers in an IdSpace according to some field.
  *
  * Example: A map from a range of Field values such as Temperature to an IdSpace
  * representing points of a dataset allows fast lookups from a temperature range
  * to points whose temperature lies in the range.
  *
  */
class SMTKMARKUP_EXPORT Index : public smtk::markup::Domain
{
public:
  smtkTypeMacro(smtk::markup::Index);
  smtkSuperclassMacro(smtk::markup::Domain);

  template<typename... Args>
  Index(Args&&... /*args*/)
  {
  }

  template<typename... Args>
  Index(smtk::string::Token name, Args&&... /*args*/)
    : smtk::markup::Domain(name)
  {
  }

  template<typename... Args>
  Index(const nlohmann::json& data, Args&&... /*args*/)
    : smtk::markup::Domain(data)
  {
  }

  ~Index() override;

protected:
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Index_h
