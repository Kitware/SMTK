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

  /// Append all the relations of kind \a k to \a result.
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
            result.push_back(typename T::value_type(c.storage(), relations[i]));
            }
          }
        else
          {
          for (std::vector<int>::iterator it = arrIt->details().begin(); it != arrIt->details().end(); ++it)
            {
            result.push_back(typename T::value_type(c.storage(), relations[*it]));
            }
          }
        }
      }
    }
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_CursorArrangementOps_h
