//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_EntityRefArrangementOps_h
#define __smtk_model_EntityRefArrangementOps_h

#include "smtk/model/ArrangementKind.h"
#include "smtk/model/EntityRef.h"

namespace smtk
{
namespace model
{

/**\brief A class to help obtain entityrefs from arrangement information.
  *
  */
class SMTKCORE_EXPORT EntityRefArrangementOps
{
public:
  static int findSimpleRelationship(const EntityRef& a, ArrangementKind k, const EntityRef& b);
  static int findOrAddSimpleRelationship(const EntityRef& a, ArrangementKind k, const EntityRef& b);

  /// Return the first relation of kind \a k as the specified entityref type \a T.
  template <typename T>
  static T firstRelation(const EntityRef& c, ArrangementKind k)
  {
    EntityPtr entRec;
    Arrangements* arr;
    if (c.checkForArrangements(k, entRec, arr))
    {
      smtk::common::UUIDArray const& relations(entRec->relations());
      for (Arrangements::iterator arrIt = arr->begin(); arrIt != arr->end(); ++arrIt)
      {
        std::vector<int>::iterator it;
        for (it = arrIt->details().begin(); it != arrIt->details().end(); ++it)
        {
          return T(c.resource(), relations[*it]);
        }
      }
    }
    return T();
  }

  /**\brief Append all the relations of kind \a k to \a result.
    *
    * This will verify that each relation is valid before
    * inserting it into \a result.
    */
  template <typename T>
  static void appendAllRelations(const EntityRef& c, ArrangementKind k, T& result)
  {
    EntityPtr entRec;
    Arrangements* arr;
    if (c.checkForArrangements(k, entRec, arr))
    {
      switch (k)
      {
        case HAS_USE:
          if (isCellEntity(entRec->entityFlags()))
          {
            appendAllCellHasUseRelations(c.resource(), entRec, arr, result);
            return;
          }
          else if (isShellEntity(entRec->entityFlags()))
          {
            appendAllShellHasUseRelations(c.resource(), entRec, arr, result);
            return;
          }
          break;
        case HAS_CELL:
          if (isUseEntity(entRec->entityFlags()))
          {
            appendAllUseHasCellRelations(c.resource(), entRec, arr, result);
            return;
          }
          break;
        default:
          break;
      }
      appendAllSimpleRelations(c.resource(), entRec, arr, result);
    }
  }

  /**\brief Helper methods used by appendAllRelations.
    */
  template <typename T>
  static void appendAllUseHasCellRelations(
    ResourcePtr resource, EntityPtr entRec, Arrangements* arr, T& result)
  {
    smtk::common::UUIDArray const& relations(entRec->relations());
    for (Arrangements::iterator arrIt = arr->begin(); arrIt != arr->end(); ++arrIt)
    {
      // Use HAS_CELL arrangements are specified as [relIdx, sense] tuples.
      int relIdx, relSense;
      if (arrIt->IndexAndSenseFromUseHasCell(relIdx, relSense) && relIdx >= 0)
      {
        typename T::value_type entry(resource, relations[relIdx]);
        if (entry.isValid())
          result.insert(result.end(), entry);
      }
    }
  }
  template <typename T>
  static void appendAllCellHasUseRelations(
    ResourcePtr resource, EntityPtr entRec, Arrangements* arr, T& result)
  {
    smtk::common::UUIDArray const& relations(entRec->relations());
    for (Arrangements::iterator arrIt = arr->begin(); arrIt != arr->end(); ++arrIt)
    {
      // Cell HAS_USE arrangements are specified as [relIdx, sense, orientation] tuples.
      int relIdx, relSense;
      Orientation relOrient;
      if (arrIt->IndexSenseAndOrientationFromCellHasUse(relIdx, relSense, relOrient) && relIdx >= 0)
      {
        typename T::value_type entry(resource, relations[relIdx]);
        if (entry.isValid())
          result.insert(result.end(), entry);
      }
    }
  }
  template <typename T>
  static void appendAllShellHasUseRelations(
    ResourcePtr resource, EntityPtr entRec, Arrangements* arr, T& result)
  {
    smtk::common::UUIDArray const& relations(entRec->relations());
    for (Arrangements::iterator arrIt = arr->begin(); arrIt != arr->end(); ++arrIt)
    {
      // Shell HAS_USE arrangements are specified as [min,max[ offset-ranges,
      // not arrays of offset values.
      int i0, i1;
      arrIt->IndexRangeFromShellHasUse(i0, i1);
      for (int i = i0; i < i1; ++i)
      {
        typename T::value_type entry(resource, relations[i]);
        if (entry.isValid())
          result.insert(result.end(), entry);
      }
    }
  }
  /// Invalidate all the relations participating in shell-has-use arrangements, clear the arrangements,
  /// and place the use records in \a result (in sequential order).
  /// Add the indices of the invalidated relations to \a rangeDetector.
  ///
  /// This can be used as a first step in rewriting loops (which must have edge-uses remain in order).
  template <typename T, typename U>
  static void popAllShellHasUseRelations(
    ResourcePtr resource, EntityPtr entRec, Arrangements* arr, T& result, U& rangeDetector)
  {
    smtk::common::UUIDArray const& relations(entRec->relations());
    for (Arrangements::iterator arrIt = arr->begin(); arrIt != arr->end(); ++arrIt)
    {
      // Shell HAS_USE arrangements are specified as [min,max[ offset-ranges,
      // not arrays of offset values.
      int i0, i1;
      arrIt->IndexRangeFromShellHasUse(i0, i1);
      for (int i = i0; i < i1; ++i)
      {
        typename T::value_type entry(resource, relations[i]);
        rangeDetector.insert(i);
        entRec->invalidateRelationByIndex(i);
        if (entry.isValid())
        {
          result.insert(result.end(), entry);
        }
      }
    }
    arr->clear();
  }
  template <typename T>
  static void appendAllSimpleRelations(
    ResourcePtr resource, EntityPtr entRec, Arrangements* arr, T& result)
  {
    smtk::common::UUIDArray const& relations(entRec->relations());
    for (Arrangements::iterator arrIt = arr->begin(); arrIt != arr->end(); ++arrIt)
    {
      std::vector<int>::iterator it;
      for (it = arrIt->details().begin(); it != arrIt->details().end(); ++it)
      {
        if (*it < 0)
          continue; // Ignore invalid indices
        typename T::value_type entry(resource, relations[*it]);
        if (entry.isValid())
        {
          result.insert(result.end(), entry);
        }
      }
    }
  }
};

// What follows are methods of EntityRef that require EntityRefArrangementOps.
// This breaks an include-dependency cycle.

template <typename T>
T EntityRef::embeddedEntities() const
{
  T result;
  EntityRefArrangementOps::appendAllRelations(*this, INCLUDES, result);
  return result;
}

template <typename T>
T EntityRef::instances() const
{
  T result;
  EntityRefArrangementOps::appendAllRelations(*this, INSTANCED_BY, result);
  return result;
}

} // namespace model
} // namespace smtk

#endif // __smtk_model_EntityRefArrangementOps_h
