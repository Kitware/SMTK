//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_arcs_Inverse_h
#define smtk_graph_arcs_Inverse_h

#include "smtk/graph/detail/TypeTraits.h"

#include <type_traits>

namespace smtk
{
namespace graph
{

/** Class if handling communicating adding/removing inverse arcs
 */
template<typename ArcType, typename = void>
class Inverse;

/** Default behavior if CRTP inheritance is not used
 */
template<>
class Inverse<void>
{
public:
  template<typename... Args>
  static bool insert(Args&&...)
  {
    return true;
  }

  template<typename... Args>
  static std::size_t erase(Args&&...)
  {
    return 0;
  }
};

/** Inverse arc handler when neither inverse is ordered
 */
template<typename ArcType>
class Inverse<
  ArcType,
  typename std::enable_if<
    detail::has_inverse<ArcType>::value &&                                               //
    detail::is_inverse_pair<ArcType, typename ArcTraits<ArcType>::InverseArcType>::value //
    >::type>
{
  using InverseArcType = typename detail::ArcInverse<ArcType>::type;
  using API = typename InverseArcType::template API<InverseArcType>;

  using arc_is_ordered = detail::is_ordered_arcs<ArcType>;
  using inverse_is_ordered = detail::is_ordered_arcs<InverseArcType>;

  static_assert(
    !(arc_is_ordered::value && inverse_is_ordered::value),
    "Implementation of Inverse<OrderedArcs<FromType,ToType>, OrderedArcs<ToType,FromType>> is not "
    "well defined."
    "This case requires an application specific implementation.");

public:
  using FromType = typename ArcTraits<InverseArcType>::FromType;
  using ToType = typename ArcTraits<InverseArcType>::ToType;

  template<class U = InverseArcType, class... Args>
  static typename std::enable_if<detail::is_ordered_arcs<U>::value, bool>::type insert(Args&&...)
  {
    throw std::logic_error("Attempting to implicitly insert an Inverse OrderedArcs. This requires "
                           "an explicit implementation");
    return false;
  }

  template<class U = InverseArcType, class... Args>
  static typename std::enable_if<!detail::is_ordered_arcs<U>::value, bool>::type insert(
    const FromType& from,
    ToType& to)
  {
    auto& iarc = API().get(from);
    // Insertion of an inverse is successful of the inverse exists.
    return iarc.insert(to, false).first != iarc.end();
  }

  std::size_t erase(const FromType& from, const ToType& to)
  {
    return API().get(from).erase(to, false);
  }
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_Inverse_h
