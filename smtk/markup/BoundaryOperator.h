//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_BoundaryOperator_h
#define smtk_markup_BoundaryOperator_h

#include "smtk/markup/Domain.h"

namespace smtk
{
namespace markup
{

/**\brief A discrete mapping from one IdSpace into another that enumerates
  *       boundaries of the domain.
  *
  * Instances of this class are intended to hold maps from cell connectivity
  * to the connectivity of their boundaries and vice-versa (from the boundary
  * connectivity to a cell side ID).
  */
class SMTKMARKUP_EXPORT BoundaryOperator : public smtk::markup::Domain
{
public:
  smtkTypeMacro(smtk::markup::BoundaryOperator);
  smtkSuperclassMacro(smtk::markup::Domain);

  BoundaryOperator() = default;
  BoundaryOperator(smtk::string::Token name);
  BoundaryOperator(const nlohmann::json& data);
  ~BoundaryOperator() override = default;

protected:
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_BoundaryOperator_h
