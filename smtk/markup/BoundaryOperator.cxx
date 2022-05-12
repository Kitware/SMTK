//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/BoundaryOperator.h"

namespace smtk
{
namespace markup
{

BoundaryOperator::BoundaryOperator(smtk::string::Token name)
  : Domain(name)
{
  // TODO: create mapping?
}

BoundaryOperator::BoundaryOperator(const nlohmann::json& data)
  : Domain(data)
{
  // TODO: Deserialize or create mapping
}

} // namespace markup
} // namespace smtk
