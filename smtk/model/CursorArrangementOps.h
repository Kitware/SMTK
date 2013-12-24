#ifndef __smtk_model_CursorArrangementOps_h
#define __smtk_model_CursorArrangementOps_h

#include "smtk/model/Cursor.h"

namespace smtk {
  namespace model {

/**\brief A class to help obtain cursors from arrangement information.
  *
  */
class SMTKCORE_EXPORT CursorArrangementOps
{
public:
  static int findSimpleRelationship(const Cursor& a, ArrangementKind k, const Cursor& b);
  static int findOrAddSimpleRelationship(const Cursor& a, ArrangementKind k, const Cursor& b);

  /// Return the first relation of kind \a k as the specified cursor type \a T.
  template<typename T>
  static T firstRelation(const Cursor& c, ArrangementKind k)
    {
    Entity* entRec;
    Arrangements* arr;
    if (c.checkForArrangements(k, entRec, arr))
      {
      smtk::util::UUIDArray const& relations(entRec->relations());
      for (Arrangements::iterator arrIt = arr->begin(); arrIt != arr->end(); ++arrIt)
        {
        std::vector<int>::iterator it;
        for (it = arrIt->details().begin(); it != arrIt->details().end(); ++it)
          {
          return T(c.storage(), relations[*it]);
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
  template<typename T>
  static void appendAllRelations(const Cursor& c, ArrangementKind k, T& result)
    {
    Entity* entRec;
    Arrangements* arr;
    if (c.checkForArrangements(k, entRec, arr))
      {
      switch (k)
        {
      case HAS_USE:
        if (isCellEntity(entRec->entityFlags()))
          {
          appendAllCellHasUseRelations(c.storage(), entRec, arr, result);
          return;
          }
        else if (isShellEntity(entRec->entityFlags()))
          {
          appendAllShellHasUseRelations(c.storage(), entRec, arr, result);
          return;
          }
        break;
      case HAS_CELL:
        if (isUseEntity(entRec->entityFlags()))
          {
          appendAllUseHasCellRelations(c.storage(), entRec, arr, result);
          return;
          }
        break;
      default:
        break;
        }
      appendAllSimpleRelations(c.storage(), entRec, arr, result);
      }
    }

  /**\brief Helper methods used by appendAllRelations.
    */
  template<typename T>
  static void appendAllUseHasCellRelations(
    StoragePtr storage, Entity* entRec, Arrangements* arr, T& result)
    {
    smtk::util::UUIDArray const& relations(entRec->relations());
    for (Arrangements::iterator arrIt = arr->begin(); arrIt != arr->end(); ++arrIt)
      {
      // Use HAS_CELL arrangements are specified as [relIdx, sense] tuples.
      int relIdx, relSense;
      if (
        arrIt->IndexAndSenseFromUseHasCell(relIdx, relSense) &&
        relIdx >= 0)
        {
        typename T::value_type entry(storage, relations[relIdx]);
        if (entry.isValid())
          result.insert(result.end(), entry);
        }
      }
    }
  template<typename T>
  static void appendAllCellHasUseRelations(
    StoragePtr storage, Entity* entRec, Arrangements* arr, T& result)
    {
    smtk::util::UUIDArray const& relations(entRec->relations());
    for (Arrangements::iterator arrIt = arr->begin(); arrIt != arr->end(); ++arrIt)
      {
      // Cell HAS_USE arrangements are specified as [relIdx, sense, orientation] tuples.
      int relIdx, relSense;
      Orientation relOrient;
      if (
        arrIt->IndexSenseAndOrientationFromCellHasUse(relIdx, relSense, relOrient) &&
        relIdx >= 0)
        {
        typename T::value_type entry(storage, relations[relIdx]);
        if (entry.isValid())
          result.insert(result.end(), entry);
        }
      }
    }
  template<typename T>
  static void appendAllShellHasUseRelations(
    StoragePtr storage, Entity* entRec, Arrangements* arr, T& result)
    {
    smtk::util::UUIDArray const& relations(entRec->relations());
    for (Arrangements::iterator arrIt = arr->begin(); arrIt != arr->end(); ++arrIt)
      {
      // Shell HAS_USE arrangements are specified as [min,max[ offset-ranges,
      // not arrays of offset values.
      int i0, i1;
      arrIt->IndexRangeFromShellHasUse(i0, i1);
      for (int i = i0; i < i1; ++i)
        {
        typename T::value_type entry(storage, relations[i]);
        if (entry.isValid())
          result.insert(result.end(), entry);
        }
      }
    }
  template<typename T>
  static void appendAllSimpleRelations(
    StoragePtr storage, Entity* entRec, Arrangements* arr, T& result)
    {
    smtk::util::UUIDArray const& relations(entRec->relations());
    for (Arrangements::iterator arrIt = arr->begin(); arrIt != arr->end(); ++arrIt)
      {
      std::vector<int>::iterator it;
      for (it = arrIt->details().begin(); it != arrIt->details().end(); ++it)
        {
        if (*it < 0) continue; // Ignore invalid indices
        typename T::value_type entry(storage, relations[*it]);
        if (entry.isValid())
          {
          result.insert(result.end(), entry);
          }
        }
      }
    }
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_CursorArrangementOps_h
