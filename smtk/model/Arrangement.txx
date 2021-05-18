//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_Arrangement_txx
#define smtk_model_Arrangement_txx

#include "smtk/model/Arrangement.h"
#include "smtk/model/Entity.h"

namespace smtk
{
namespace model
{

/// A helper to extract the relationship from an arrangement that stores only an index.
template<bool (Arrangement::*M)(int&) const>
struct Arrangement::IndexHelper
{
  bool operator()(smtk::common::UUIDArray& rels, const EntityPtr entity, const Arrangement& arr)
    const
  {
    if (entity)
    {
      int idx;
      if ((arr.*M)(idx))
        if (idx >= 0 && idx < static_cast<int>(entity->relations().size()))
          rels.push_back(entity->relations()[idx]);
    }
    return !rels.empty();
  }
  bool operator()(std::vector<int>& relIdxs, const EntityPtr entity, const Arrangement& arr) const
  {
    if (entity)
    {
      int idx;
      if ((arr.*M)(idx))
        if (idx >= 0 && idx < static_cast<int>(entity->relations().size()))
          relIdxs.push_back(idx);
    }
    return !relIdxs.empty();
  }
};

/// A helper to extract the relationship from an arrangement that stores an index and sense.
template<bool (Arrangement::*M)(int&, int&) const>
struct Arrangement::IndexAndSenseHelper
{
  bool operator()(smtk::common::UUIDArray& rels, const EntityPtr entity, const Arrangement& arr)
    const
  {
    if (entity)
    {
      int idx, sense;
      if ((arr.*M)(idx, sense))
        if (idx >= 0 && idx < static_cast<int>(entity->relations().size()))
          rels.push_back(entity->relations()[idx]);
    }
    return !rels.empty();
  }
  bool operator()(std::vector<int>& relIdxs, const EntityPtr entity, const Arrangement& arr) const
  {
    if (entity)
    {
      int idx, sense;
      if ((arr.*M)(idx, sense))
        if (idx >= 0 && idx < static_cast<int>(entity->relations().size()))
          relIdxs.push_back(idx);
    }
    return !relIdxs.empty();
  }
};

/// A helper to extract relationships from an arrangement that stores an index range.
template<bool (Arrangement::*M)(int&, int&) const>
struct Arrangement::IndexRangeHelper
{
  bool operator()(smtk::common::UUIDArray& rels, const EntityPtr entity, const Arrangement& arr)
    const
  {
    if (entity)
    {
      int ibeg, iend;
      if ((arr.*M)(ibeg, iend))
        for (; ibeg < iend; ++ibeg)
          if (ibeg >= 0 && ibeg < static_cast<int>(entity->relations().size()))
            rels.push_back(entity->relations()[ibeg]);
    }
    return !rels.empty();
  }
  bool operator()(std::vector<int>& relIdxs, const EntityPtr entity, const Arrangement& arr) const
  {
    if (entity)
    {
      int ibeg, iend;
      if ((arr.*M)(ibeg, iend))
        for (; ibeg < iend; ++ibeg)
          if (ibeg >= 0 && ibeg < static_cast<int>(entity->relations().size()))
            relIdxs.push_back(ibeg);
    }
    return !relIdxs.empty();
  }
};

/// A helper to extract the relationship from an arrangement that stores an index, sense, and orientation.
template<bool (Arrangement::*M)(int&, int&, Orientation&) const>
struct Arrangement::IndexSenseAndOrientationHelper
{
  bool operator()(smtk::common::UUIDArray& rels, const EntityPtr entity, const Arrangement& arr)
    const
  {
    if (entity)
    {
      int idx, sense;
      Orientation orient;
      if ((arr.*M)(idx, sense, orient))
        if (idx >= 0 && idx < static_cast<int>(entity->relations().size()))
          rels.push_back(entity->relations()[idx]);
    }
    return !rels.empty();
  }
  bool operator()(std::vector<int>& relIdxs, const EntityPtr entity, const Arrangement& arr) const
  {
    if (entity)
    {
      int idx, sense;
      Orientation orient;
      if ((arr.*M)(idx, sense, orient))
        if (idx >= 0 && idx < static_cast<int>(entity->relations().size()))
          relIdxs.push_back(idx);
    }
    return !relIdxs.empty();
  }
};

} // namespace model
} // namespace smtk

#endif
