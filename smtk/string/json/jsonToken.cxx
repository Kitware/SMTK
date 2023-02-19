//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/string/json/jsonToken.h"

namespace smtk
{
namespace string
{

void to_json(json& j, const Token& t)
{
  if (t.hasData())
  {
    j = t.data();
  }
  else
  {
    j = t.id();
  }
}

void from_json(const json& j, Token& t)
{
  if (j.is_string())
  {
    t = Token(j.get<std::string>());
  }
  else
  {
    t = Token::fromHash(j.get<Hash>());
  }
}

} // namespace string
} // namespace smtk
