#ifndef __smtk_model_Arrangement_h
#define __smtk_model_Arrangement_h

#include "smtk/model/ArrangementKind.h"
#include "smtk/model/BRepModel.h"
#include "smtk/util/UUID.h"

#include "smtk/options.h" // for SMTK_HASH_STORAGE
#ifdef SMTK_HASH_STORAGE
#  include "sparsehash/sparse_hash_map"
#endif // SMTK_HASH_STORAGE

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

  static Arrangement CellHasUseWithIndexSenseAndOrientation(int relationIdx, int sense, Orientation o);
  static Arrangement CellEmbeddedInEntityWithIndex(int relationIdx);
  static Arrangement CellIncludesEntityWithIndex(int relationIdx);
  static Arrangement CellHasShellWithIndex(int relationIdx);
  static Arrangement UseHasCellWithIndexAndSense(int relationIdx, int sense);
  static Arrangement UseHasShellWithIndex(int relationIdx);
  static Arrangement UseOrShellIncludesShellWithIndex(int relationIdx);
  static Arrangement ShellHasCellWithIndex(int relationIdx);
  static Arrangement ShellHasUseWithIndexRange(int relationBegin, int relationEnd);
  static Arrangement ShellEmbeddedInUseOrShellWithIndex(int relationIdx);

  bool IndexSenseAndOrientationFromCellHasUse(int& relationIdx, int& sense, Orientation& orient) const;
  bool IndexFromCellEmbeddedInEntity(int& relationIdx) const;
  bool IndexFromCellIncludesEntity(int& relationIdx) const;
  bool IndexFromCellHasShell(int& relationIdx) const;
  bool IndexAndSenseFromUseHasCell(int& relationIdx, int& sense) const;
  bool IndexFromUseHasShell(int& relationIdx) const;
  bool IndexFromUseOrShellIncludesShell(int& relationIdx) const;
  bool IndexFromShellHasCell(int& relationIdx) const;
  bool IndexRangeFromShellHasUse(int& relationBegin, int& relationEnd) const;
  bool IndexFromShellEmbeddedInUseOrShell(int& relationIdx) const;

  static Arrangement SimpleIndex(int relationIdx);
  bool IndexFromSimple(int& relationIdx) const;

protected:
  std::vector<int> m_details; // Kind-dependent specification of the arrangement.
};

/// A vector of Arrangements is associated to each Storage entity.
typedef std::vector<Arrangement> Arrangements;
/// A map holding Arrangements of different ArrangementKinds.
typedef std::map<ArrangementKind,Arrangements> KindsToArrangements;
#ifdef SMTK_HASH_STORAGE
/// Each Storage entity's UUID is mapped to a vector of Arrangment instances.
typedef google::sparse_hash_map<smtk::util::UUID,KindsToArrangements> UUIDsToArrangements;
/// An iterator referencing a (UUID,KindsToArrangements)-tuple.
typedef google::sparse_hash_map<smtk::util::UUID,KindsToArrangements>::iterator UUIDWithArrangementDictionary;
#else
/// Each Storage entity's UUID is mapped to a vector of Arrangment instances.
typedef std::map<smtk::util::UUID,KindsToArrangements> UUIDsToArrangements;
/// An iterator referencing a (UUID,KindsToArrangements)-tuple.
typedef std::map<smtk::util::UUID,KindsToArrangements>::iterator UUIDWithArrangementDictionary;
#endif // SMTK_HASH_STORAGE
/// An iterator referencing an (ArrangementKind,Arrangements)-tuple.
typedef std::map<ArrangementKind,Arrangements>::iterator ArrangementKindWithArrangements;

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Arrangement_h
