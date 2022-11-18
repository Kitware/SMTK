//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_IdNature_h
#define smtk_markup_IdNature_h

#include "smtk/markup/Exports.h"

#include "smtk/string/Token.h"

namespace smtk
{
namespace markup
{

/// The nature of how identifiers in an instance of AssignedIds are used.
enum IdNature
{
  Primary,      //!< Identifiers are exclusively owned by the \a component.
  Referential,  //!< Identifiers are not owned – only referenced – by the \a component.
  NonExclusive, //!< Ownership of identifiers is shared with other components.
  Unassigned    //!< Identifiers are not reserved for use.
};

/// Return a Nature enumerant from a string token.
///
/// Accepted string tokens include upper-camel- and snake-case variants
/// of the enumerants: "Primary", "primary", "Referential", "referential",
/// "NonExclusive", "non_exclusive", "Unassigned", or "unassigned".
/// Any other token will default to Nature::Unassigned.
IdNature SMTKMARKUP_EXPORT natureEnumerant(smtk::string::Token natureToken);

/// Return a string token for a Nature enumerant.
smtk::string::Token SMTKMARKUP_EXPORT natureToken(IdNature natureEnumerant);

} // namespace markup
} // namespace smtk

#endif // smtk_markup_IdNature_h
