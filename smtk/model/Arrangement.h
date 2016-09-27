//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Arrangement_h
#define __smtk_model_Arrangement_h

#include "smtk/Options.h" // for SMTK_HASH_STORAGE

#include "smtk/common/UUID.h"

#include "smtk/model/ArrangementKind.h"
#include "smtk/model/Entity.h"

#ifdef SMTK_HASH_STORAGE
#  if defined(_MSC_VER) // Visual studio
#    pragma warning (push)
#    pragma warning (disable : 4996)  // Overeager "unsafe" parameter check
#  endif
#  include "sparsehash/sparse_hash_map"
#  if defined(_MSC_VER) // Visual studio
#    pragma warning (pop)
#  endif
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

  static Arrangement Construct(EntityTypeBits t, ArrangementKind k, int relationIdx, int sense, Orientation o);
  //static Arrangement ConstructParent(EntityTypeBits t, ArrangementKind k, int childIdx, int sense, Orientation o);
  //static Arrangement ConstructChild(EntityTypeBits t, ArrangementKind k, int parentIdx, int sense, Orientation o);

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
  static Arrangement InstanceInstanceOfWithIndex(int relationIdx);
  static Arrangement EntityInstancedByWithIndex(int relationIdx);
  static Arrangement EntitySupersetOfWithIndex(int relationIdx);
  static Arrangement EntitySubsetOfWithIndex(int relationIdx);

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
  bool IndexFromInstanceInstanceOf(int& relationIdx) const;
  bool IndexFromEntityInstancedBy(int& relationIdx) const;
  bool IndexFromEntitySupersetOf(int& relationIdx) const;
  bool IndexFromEntitySubsetOf(int& relationIdx) const;

  static Arrangement SimpleIndex(int relationIdx);
  bool IndexFromSimple(int& relationIdx) const;

  bool relations(smtk::common::UUIDArray& relsOut, const Entity* ent, ArrangementKind k) const;
  bool relationIndices(std::vector<int>& relsOut, const Entity* ent, ArrangementKind k) const;

  /// A helper to extract the relationship from an arrangement that stores only an index.
  template<bool (Arrangement::*M)(int&) const>
  struct IndexHelper
  {
    bool operator () (
      smtk::common::UUIDArray& rels, const Entity* entity, const Arrangement& arr) const
      {
      if (entity)
        {
        int idx;
        if ((arr.*M)(idx))
          if (idx >= 0 && idx < static_cast<int>(entity->relations().size()))
            rels.push_back(entity->relations()[idx]);
        }
      return rels.empty() ? false : true;
      }
    bool operator () (
      std::vector<int>& relIdxs, const Entity* entity, const Arrangement& arr) const
      {
      if (entity)
        {
        int idx;
        if ((arr.*M)(idx))
          if (idx >= 0 && idx < static_cast<int>(entity->relations().size()))
            relIdxs.push_back(idx);
        }
      return relIdxs.empty() ? false : true;
      }
  };

  /// A helper to extract the relationship from an arrangement that stores an index and sense.
  template<bool (Arrangement::*M)(int&, int&) const>
  struct IndexAndSenseHelper
  {
    bool operator () (
      smtk::common::UUIDArray& rels, const Entity* entity, const Arrangement& arr) const
      {
      if (entity)
        {
        int idx, sense;
        if ((arr.*M)(idx, sense))
          if (idx >= 0 && idx < static_cast<int>(entity->relations().size()))
            rels.push_back(entity->relations()[idx]);
        }
      return rels.empty() ? false : true;
      }
    bool operator () (
      std::vector<int>& relIdxs, const Entity* entity, const Arrangement& arr) const
      {
      if (entity)
        {
        int idx, sense;
        if ((arr.*M)(idx, sense))
          if (idx >= 0 && idx < static_cast<int>(entity->relations().size()))
            relIdxs.push_back(idx);
        }
      return relIdxs.empty() ? false : true;
      }
  };

  /// A helper to extract relationships from an arrangement that stores an index range.
  template<bool (Arrangement::*M)(int&, int&) const>
  struct IndexRangeHelper
  {
    bool operator () (
      smtk::common::UUIDArray& rels, const Entity* entity, const Arrangement& arr) const
      {
      if (entity)
        {
        int ibeg, iend;
        if ((arr.*M)(ibeg, iend))
          for (; ibeg < iend; ++ibeg)
            if (ibeg >= 0 && ibeg < static_cast<int>(entity->relations().size()))
              rels.push_back(entity->relations()[ibeg]);
        }
      return rels.empty() ? false : true;
      }
    bool operator () (
      std::vector<int>& relIdxs, const Entity* entity, const Arrangement& arr) const
      {
      if (entity)
        {
        int ibeg, iend;
        if ((arr.*M)(ibeg, iend))
          for (; ibeg < iend; ++ibeg)
            if (ibeg >= 0 && ibeg < static_cast<int>(entity->relations().size()))
              relIdxs.push_back(ibeg);
        }
      return relIdxs.empty() ? false : true;
      }
  };

  /// A helper to extract the relationship from an arrangement that stores an index, sense, and orientation.
  template<bool (Arrangement::*M)(int&, int&, Orientation&) const>
  struct IndexSenseAndOrientationHelper
  {
    bool operator () (
      smtk::common::UUIDArray& rels, const Entity* entity, const Arrangement& arr) const
      {
      if (entity)
        {
        int idx, sense;
        Orientation orient;
        if ((arr.*M)(idx, sense, orient))
          if (idx >= 0 && idx < static_cast<int>(entity->relations().size()))
            rels.push_back(entity->relations()[idx]);
        }
      return rels.empty() ? false : true;
      }
    bool operator () (
      std::vector<int>& relIdxs, const Entity* entity, const Arrangement& arr) const
      {
      if (entity)
        {
        int idx, sense;
        Orientation orient;
        if ((arr.*M)(idx, sense, orient))
          if (idx >= 0 && idx < static_cast<int>(entity->relations().size()))
            relIdxs.push_back(idx);
        }
      return relIdxs.empty() ? false : true;
      }
  };

  typedef IndexSenseAndOrientationHelper<&Arrangement::IndexSenseAndOrientationFromCellHasUse> CellHasUseRelationHelper;
  typedef IndexHelper<&Arrangement::IndexFromCellEmbeddedInEntity> CellEmbeddedInEntityRelationHelper;
  typedef IndexHelper<&Arrangement::IndexFromCellIncludesEntity> CellIncludesEntityRelationHelper;
  typedef IndexHelper<&Arrangement::IndexFromCellHasShell> CellHasShellRelationHelper;
  typedef IndexAndSenseHelper<&Arrangement::IndexAndSenseFromUseHasCell> UseHasCellRelationHelper;
  typedef IndexHelper<&Arrangement::IndexFromUseHasShell> UseHasShellRelationHelper;
  typedef IndexHelper<&Arrangement::IndexFromUseOrShellIncludesShell> UseOrShellIncludesShellRelationHelper;
  typedef IndexHelper<&Arrangement::IndexFromShellHasCell> ShellHasCellRelationHelper;
  typedef IndexRangeHelper<&Arrangement::IndexRangeFromShellHasUse> ShellHasUseRelationHelper;
  typedef IndexHelper<&Arrangement::IndexFromShellEmbeddedInUseOrShell> ShellEmbeddedInUseOrShellRelationHelper;
  typedef IndexHelper<&Arrangement::IndexFromInstanceInstanceOf> InstanceInstanceOfRelationHelper;
  typedef IndexHelper<&Arrangement::IndexFromEntityInstancedBy> InstanceInstancedByRelationHelper;
  typedef IndexHelper<&Arrangement::IndexFromEntitySupersetOf> EntitySupersetOfRelationHelper;
  typedef IndexHelper<&Arrangement::IndexFromEntitySubsetOf> EntitySubsetOfRelationHelper;


protected:
  std::vector<int> m_details; // Kind-dependent specification of the arrangement.
};

/**\brief A simple structure that robustly references an arrangement.
  *
  * This is more robust than pointers or iterators into instances of
  * UUIDsToArrangements objects, since these are invalidated by
  * modifications to arrangements.
  */
class ArrangementReference
{
public:
  /// Construct a valid reference.
  ArrangementReference(
    const smtk::common::UUID& entId, ArrangementKind k, int idx)
    : entityId(entId), kind(k), index(idx)
    { }
  /// Construct an invalid reference.
  ArrangementReference()
    : kind(KINDS_OF_ARRANGEMENTS), index(-1)
    { }

  /// Indicate whether a reference is valid or not:
  bool isValid() const
    {
    return this->kind == KINDS_OF_ARRANGEMENTS || this->index < 0 || !this->entityId;
    }

  smtk::common::UUID entityId; //!< The ID of the entity on which the arrangement is defined.
  ArrangementKind kind;      //!< The kind of the arrangement.
  int index;                 //!< The index of the arrangement.
};

/// A vector of Arrangements is associated to each Manager entity.
typedef std::vector<Arrangement> Arrangements;
/// A map holding Arrangements of different ArrangementKinds.
typedef std::map<ArrangementKind,Arrangements> KindsToArrangements;
#ifdef SMTK_HASH_STORAGE
/// Each Manager entity's UUID is mapped to a vector of Arrangment instances.
typedef google::sparse_hash_map<smtk::common::UUID,KindsToArrangements> UUIDsToArrangements;
/// An iterator referencing a (UUID,KindsToArrangements)-tuple.
typedef google::sparse_hash_map<smtk::common::UUID,KindsToArrangements>::iterator UUIDWithArrangementDictionary;
#else
/// Each Manager entity's UUID is mapped to a vector of Arrangment instances.
typedef std::map<smtk::common::UUID,KindsToArrangements> UUIDsToArrangements;
/// An iterator referencing a (UUID,KindsToArrangements)-tuple.
typedef std::map<smtk::common::UUID,KindsToArrangements>::iterator UUIDWithArrangementDictionary;
#endif // SMTK_HASH_STORAGE
/// An iterator referencing an (ArrangementKind,Arrangements)-tuple.
typedef std::map<ArrangementKind,Arrangements>::iterator ArrangementKindWithArrangements;
/// An array of ArrangementReference objects used, for instance, to enumerate inverse relations.
typedef std::vector<ArrangementReference> ArrangementReferences;

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Arrangement_h
