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
        for (std::vector<int>::iterator it = arrIt->details().begin(); it != arrIt->details().end(); ++it)
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
      smtk::util::UUIDArray const& relations(entRec->relations());
      for (Arrangements::iterator arrIt = arr->begin(); arrIt != arr->end(); ++arrIt)
        {
        if (c.isShell() && arrIt->details().size() == 2)
          {
          // Shell HAS_USE arrangements are specified as [min,max[ offset-ranges,
          // not arrays of offset values.
          for (int i = arrIt->details()[0]; i < arrIt->details()[1]; ++i)
            {
            typename T::value_type entry(c.storage(), relations[i]);
            if (entry.isValid())
              result.insert(result.end(), entry);
            }
          }
        else
          {
          for (std::vector<int>::iterator it = arrIt->details().begin(); it != arrIt->details().end(); ++it)
            {
            typename T::value_type entry(c.storage(), relations[*it]);
            if (entry.isValid())
              result.insert(result.end(), entry);
            }
          }
        }
      }
    }
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_CursorArrangementOps_h
