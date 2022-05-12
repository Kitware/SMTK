//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/IdNature.h"

namespace smtk
{
namespace markup
{

IdNature natureEnumerant(smtk::string::Token natureToken)
{
  using namespace smtk::string::literals;
  IdNature result;
  switch (natureToken.id())
  {
    case "Primary"_hash: // fall through
    case "primary"_hash:
      result = IdNature::Primary;
      break;
    case "Referential"_hash: // fall through
    case "referential"_hash:
      result = IdNature::Referential;
      break;
    case "NonExclusive"_hash: // fall through
    case "non_exclusive"_hash:
      result = IdNature::NonExclusive;
      break;
    case "Unassigned"_hash: // fall through
    case "unassigned"_hash: // fall through
    default:
      result = IdNature::Unassigned;
      break;
  }
  return result;
}

smtk::string::Token natureToken(IdNature natureEnumerant)
{
  using namespace smtk::string::literals;

  switch (natureEnumerant)
  {
    case IdNature::Primary:
      return "primary"_token;
      break;
    case IdNature::Referential:
      return "referential"_token;
      break;
    case IdNature::NonExclusive:
      return "non_exclusive"_token;
      break;
    default:
      break;
  }
  return "unassigned"_token;
}

} // namespace markup
} // namespace smtk
