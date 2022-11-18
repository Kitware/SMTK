//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/Domain.h"

#include "smtk/string/json/jsonToken.h"

namespace smtk
{
namespace markup
{

Domain::Domain(smtk::string::Token name)
  : m_name(name)
{
}

Domain::Domain(const nlohmann::json& data)
  : m_name(data.at("name").get<smtk::string::Token>())
{
}

} // namespace markup
} // namespace smtk
