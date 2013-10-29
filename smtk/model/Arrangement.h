#ifndef __smtk_model_Arrangement_h
#define __smtk_model_Arrangement_h

#include "smtk/model/BRepModel.h"
#include "smtk/util/UUID.h"

#include <map>
#include <string>
#include <vector>

namespace smtk {
  namespace model {

// === WARNING === If you change this enum, also update string names in Arrangement.cxx!
/// Specification of how a cell's relations are arranged.
enum ArrangementKind {
  INCLUSION,   //!< How another cell is contained in the interior of this cell.
  SHELL,       //!< How this cell is bounded by cells of lower dimension
  USE,         //!< How this cell's shells are combined into a single orientation for use by bordant cells.
  SENSE,       //!< How this cell participates in the boundary of a higher-dimensional cell.
  EMBEDDING,   //!< How this cell is embedded in the interior of a cell of higher dimension
  KINDS_OF_ARRANGEMENTS //!< The number of different kinds of arrangement relationships enumerated here.
};

//extern const char* ArrangementKindName[KINDS_OF_ARRANGEMENTS + 1];
//extern const char* ArrangementKindAbbr[KINDS_OF_ARRANGEMENTS + 1];
// === WARNING === If you change this enum, also update string names in Arrangement.cxx!

ArrangementKind ArrangementKindFromName(const std::string& name);
std::string NameForArrangementKind(ArrangementKind k);

ArrangementKind ArrangementKindFromAbbreviation(const std::string& abbr);
std::string AbbreviationForArrangementKind(ArrangementKind k);

/**\brief Store an arrangement of solid model entities.
  *
  * Note that arrangements may be some combination of
  * permutations, combinations, or hierarchies.
  * This structure provides storage for any type of arrangement via a vector of
  * integers. These integers may be the sense in which a cell
  * is used or offsets into a cell's relations or arrangements.
  *
  * See the documentation of ArrangementKind for specifics.
  */
struct SMTKCORE_EXPORT Arrangement
{
  std::vector<int> details; // Kind-dependent specification of the arrangement.
};

/// A vector of Arrangements is associated to each ModelBody entity.
typedef std::vector<Arrangement> Arrangements;
/// A map holding Arrangements of different ArrangementKinds.
typedef std::map<ArrangementKind,Arrangements> KindsToArrangements;
/// Each ModelBody entity's UUID is mapped to a vector of Arrangment instances.
typedef std::map<smtk::util::UUID,KindsToArrangements> UUIDsToArrangements;
/// An iterator referencing a (UUID,KindsToArrangements)-tuple.
typedef std::map<smtk::util::UUID,KindsToArrangements>::iterator UUIDWithArrangementDictionary;
/// An iterator referencing an (ArrangementKind,Arrangements)-tuple.
typedef std::map<ArrangementKind,Arrangements>::iterator ArrangementKindWithArrangements;

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Arrangement_h
