#ifndef __smtk_model_Arrangement_h
#define __smtk_model_Arrangement_h

#include "smtk/model/BRepModel.h"
#include "smtk/util/UUID.h"

#include "sparsehash/sparse_hash_map"

#include <map>
#include <string>
#include <vector>

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
  INCLUDES,   //!< How another cell is contained in the interior of this cell.
  HAS_CELL,    //!< How a use or shell is related to its cell.
  HAS_SHELL,   //!< How this cell is bounded by cells of lower dimension or how a use participates in a shell.
  HAS_USE,     //!< How this cell's shells are combined into a single orientation for use by bordant cells.
  SENSE,       //!< How this cell participates in the boundary of a higher-dimensional cell.
  EMBEDDED_IN,   //!< How this cell is embedded in the interior of a cell of higher dimension
  KINDS_OF_ARRANGEMENTS //!< The number of different kinds of arrangement relationships enumerated here.
};

//extern const char* ArrangementKindName[KINDS_OF_ARRANGEMENTS + 1];
//extern const char* ArrangementKindAbbr[KINDS_OF_ARRANGEMENTS + 1];
// === WARNING === If you change this enum, also update string names in Arrangement.cxx!

SMTKCORE_EXPORT ArrangementKind ArrangementKindFromName(const std::string& name);
SMTKCORE_EXPORT std::string NameForArrangementKind(ArrangementKind k);

SMTKCORE_EXPORT ArrangementKind ArrangementKindFromAbbreviation(const std::string& abbr);
SMTKCORE_EXPORT std::string AbbreviationForArrangementKind(ArrangementKind k);

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
class SMTKCORE_EXPORT Arrangement
{
public:
  std::vector<int>& details()
    { return this->m_details; }
  std::vector<int> const& details() const
    { return this->m_details; }

  static Arrangement CellHasUseWithIndexAndSense(int relationIdx, int sense);
  static Arrangement UseHasCellWithIndexAndSense(int relationIdx, int sense);

protected:
  std::vector<int> m_details; // Kind-dependent specification of the arrangement.
};

/// A vector of Arrangements is associated to each Storage entity.
typedef std::vector<Arrangement> Arrangements;
/// A map holding Arrangements of different ArrangementKinds.
typedef std::map<ArrangementKind,Arrangements> KindsToArrangements;
/// Each Storage entity's UUID is mapped to a vector of Arrangment instances.
typedef google::sparse_hash_map<smtk::util::UUID,KindsToArrangements> UUIDsToArrangements;
/// An iterator referencing a (UUID,KindsToArrangements)-tuple.
typedef google::sparse_hash_map<smtk::util::UUID,KindsToArrangements>::iterator UUIDWithArrangementDictionary;
/// An iterator referencing an (ArrangementKind,Arrangements)-tuple.
typedef std::map<ArrangementKind,Arrangements>::iterator ArrangementKindWithArrangements;

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Arrangement_h
