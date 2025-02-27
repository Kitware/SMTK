//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_core_Handle_h
#define smtk_mesh_core_Handle_h

#include "smtk/CoreExports.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <boost/icl/interval_set.hpp>
SMTK_THIRDPARTY_POST_INCLUDE

#include <functional>
#include <ostream>

namespace smtk
{
namespace mesh
{
typedef std::ptrdiff_t EntityId;
typedef std::size_t Handle;
} // namespace mesh
} // namespace smtk

// TODO: use extern template declaration to prevent consuming libraries from
// generating these template specializations.
template class SMTKCORE_EXPORT boost::icl::closed_interval<smtk::mesh::Handle>;
template class SMTKCORE_EXPORT boost::icl::
  interval_set<smtk::mesh::Handle, std::less, boost::icl::closed_interval<smtk::mesh::Handle>>;

namespace smtk
{
namespace mesh
{
typedef boost::icl::closed_interval<Handle> HandleInterval;
typedef boost::icl::interval_set<Handle, std::less, HandleInterval> HandleRange;

typedef decltype(boost::icl::elements_begin(
  std::declval<const HandleRange>())) const_element_iterator;

/// Return an iterator to the first element in the range
SMTKCORE_EXPORT const_element_iterator rangeElementsBegin(const HandleRange&);

/// Return an iterator to the last element in the range
SMTKCORE_EXPORT const_element_iterator rangeElementsEnd(const HandleRange&);

/// Given a handle range and an index \a i, return the i-th handle in the range
SMTKCORE_EXPORT Handle rangeElement(const HandleRange&, std::size_t);

/// Return true if the handle is contained within the handle range
SMTKCORE_EXPORT bool rangeContains(const HandleRange&, Handle);

/// Return true if the handle interval is contained within the handle range
SMTKCORE_EXPORT bool rangeContains(const HandleRange&, const HandleInterval&);

/// Return true if the second handle range is contained within the first
/// handle range
SMTKCORE_EXPORT bool rangeContains(const HandleRange& super, const HandleRange& sub);

/// Return the element index of a handle value
SMTKCORE_EXPORT std::size_t rangeIndex(const HandleRange&, Handle);

/// Return the number of intervals in the range
SMTKCORE_EXPORT std::size_t rangeIntervalCount(const HandleRange&);

/// Determine whether two ranges are equal
SMTKCORE_EXPORT bool rangesEqual(const HandleRange&, const HandleRange&);
} // namespace mesh
} // namespace smtk

SMTKCORE_EXPORT std::ostream& operator<<(std::ostream&, const smtk::mesh::HandleRange&);

#endif
