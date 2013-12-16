#ifndef __smtk_model_Arrangement_h
#define __smtk_model_Arrangement_h

#include "smtk/model/ArrangementKind.h"
#include "smtk/model/BRepModel.h"
#include "smtk/util/UUID.h"

#include "sparsehash/sparse_hash_map"

#include <map>
#include <vector>

namespace smtk {
  namespace model {

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
  ///@{
  /**\brief Access to the integer vector defining an arrangement.
    */
  std::vector<int>& details()
    { return this->m_details; }
  std::vector<int> const& details() const
    { return this->m_details; }
  ///@}

  static Arrangement CellHasUseWithIndexAndSense(int relationIdx, int sense);
  static Arrangement UseHasCellWithIndexAndSense(int relationIdx, int sense);
  static Arrangement CellEmbeddedInEntityWithIndex(int relationIdx);
  static Arrangement ShellHasCellWithIndex(int relationIdx);
  static Arrangement ShellHasUseWithIndexRange(int relationBegin, int relationEnd);

  bool IndexAndSenseFromCellHasUse(int& relationIdx, int& sense);
  bool IndexAndSenseFromUseHasCell(int& relationIdx, int& sense);
  bool IndexFromCellEmbeddedInEntity(int& relationIdx);
  bool IndexFromShellHasCell(int& relationIdx);
  bool IndexRangeFromShellHasUse(int& relationBegin, int& relationEnd);

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
