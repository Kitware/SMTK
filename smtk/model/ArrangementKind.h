#ifndef __smtk_model_ArrangementKind_h
#define __smtk_model_ArrangementKind_h

#include "smtk/SMTKCoreExports.h"

#include <string>

namespace smtk {
  namespace model {

/**\brief Constants that describe a cell-use's sense relative to its parent cell.
  *
  * These only have semantic meaning for edges and faces.
  * Vertices may have any number of senses depending on the number of
  * regions they border.
  */
enum CellUseSenses {
  NEGATIVE_SENSE = 0,
  POSITIIVE_SENSE = 1,
};

// === WARNING === If you change this enum, also update string names in Arrangement.cxx!
/// Specification of how a cell's relations are arranged.
enum ArrangementKind {
  INCLUDES,    //!< How another cell is contained in the interior of this cell.
  HAS_CELL,    //!< How a use or shell is related to its cell.
  HAS_SHELL,   //!< How this cell is bounded by cells of lower dimension or how a use participates in a shell.
  HAS_USE,     //!< How this cell's shells are combined into a single orientation for use by bordant cells.
  SENSE,       //!< How this cell participates in the boundary of a higher-dimensional cell.
  EMBEDDED_IN, //!< How this cell is embedded in the interior of a cell of higher dimension
  KINDS_OF_ARRANGEMENTS //!< The number of different kinds of arrangement relationships enumerated here.
};

//extern const char* ArrangementKindName[KINDS_OF_ARRANGEMENTS + 1];
//extern const char* ArrangementKindAbbr[KINDS_OF_ARRANGEMENTS + 1];
// === WARNING === If you change this enum, also update string names in Arrangement.cxx!

SMTKCORE_EXPORT ArrangementKind ArrangementKindFromName(const std::string& name);
SMTKCORE_EXPORT std::string NameForArrangementKind(ArrangementKind k);

SMTKCORE_EXPORT ArrangementKind ArrangementKindFromAbbreviation(const std::string& abbr);
SMTKCORE_EXPORT std::string AbbreviationForArrangementKind(ArrangementKind k);

  } // model namespace
} // smtk namespace

#endif // __smtk_model_ArrangementKind_h
