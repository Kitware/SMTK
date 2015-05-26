//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_ArrangementKind_h
#define __smtk_model_ArrangementKind_h

#include "smtk/model/EntityTypeBits.h"

#include <string>

namespace smtk {
  namespace model {

/**\brief Constants that describe a cell-use's orientation relative to its parent cell.
  *
  * These only have semantic meaning for edges and faces.
  *
  * Note that SMTK makes a distinction between the *orientation* of a cell-use
  * and the *sense* of a cell-use.
  * The orientation indicates whether the surface normal or curve direction
  * opposes or matches the cell's.
  * The *sense* is used to indicate the context in which the use occurs
  * (e.g., an edge may participate in multiple face loops, and will have
  * a separate edge-use for each loop).
  *
  * Also, note that unlike some solid modelers, SMTK composes k-shells out
  * of cell-uses. This means that each k-shell is used *exactly* once
  * because it may not have a NEGATIVE orientation relative to its cell-uses.
  * If it did, it would be composed with oppositely-oriented cell-uses.
  */
enum Orientation {
  NEGATIVE = -1,  //!< The entity is oriented opposite to the underlying geometry
  POSITIVE = +1,  //!< The entity is codirectional with the underlying geometry
  UNDEFINED = 0   //!< The relative orientation is unknown or unknowable.
};

// === WARNING === If you change this enum, also update string names in Arrangement.cxx!
/// Specification of how a cell's relations are arranged.
enum ArrangementKind {
  // Enums specific to cell, use, and shell relationships:
  INCLUDES,     //!< How another cell is contained in the interior of this cell.
  HAS_CELL,     //!< How a use or shell is related to its cell.
  HAS_SHELL,    //!< How this cell is bounded by cells of lower dimension or how a use participates in a shell.
  HAS_USE,      //!< How this cell's shells are combined into a single orientation for use by bordant cells.
  EMBEDDED_IN,  //!< How this cell is embedded in the interior of a cell of higher dimension
  // Enums specific to group relationships:
  SUBSET_OF,    //!< This entity is a subset of the related entity.
  SUPERSET_OF,  //!< This entity is a superset of the related entity.
  // Enums specific to instance relationships:
  INSTANCE_OF,  //!< This entity is an instance of the related entity.
  INSTANCED_BY, //!< This entity has an instance (duplicate) that is the related entity.
  //
  KINDS_OF_ARRANGEMENTS //!< The number of different kinds of arrangement relationships enumerated here.
};

//extern const char* ArrangementKindName[KINDS_OF_ARRANGEMENTS + 1];
//extern const char* ArrangementKindAbbr[KINDS_OF_ARRANGEMENTS + 1];
// === WARNING === If you change this enum, also update string names in Arrangement.cxx!

SMTKCORE_EXPORT ArrangementKind ArrangementKindFromName(const std::string& name);
SMTKCORE_EXPORT std::string NameForArrangementKind(ArrangementKind k);

SMTKCORE_EXPORT ArrangementKind ArrangementKindFromAbbreviation(const std::string& abbr);
SMTKCORE_EXPORT std::string AbbreviationForArrangementKind(ArrangementKind k);

SMTKCORE_EXPORT ArrangementKind Dual(EntityTypeBits entType, ArrangementKind k);

  } // model namespace
} // smtk namespace

#endif // __smtk_model_ArrangementKind_h
