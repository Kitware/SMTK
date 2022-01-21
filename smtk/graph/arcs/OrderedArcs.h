//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_OrderedArcs_h
#define smtk_graph_OrderedArcs_h

#include <functional>
#include <type_traits>
#include <vector>

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/WeakReferenceWrapper.h"

#include "smtk/graph/detail/TypeTraits.h"

namespace smtk
{
namespace graph
{

/// An ordered collection of arcs of the same type.
///
/// All arcs must have components with the same origin and destination types.
/// Furthermore, the arcs are ordered so that they are reported or visited in a
/// consistent, user-specified order.
template<typename from_type, typename to_type>
class SMTK_ALWAYS_EXPORT OrderedArcs
{
public:
  typedef from_type FromType;
  typedef to_type ToType;
  typedef std::vector<smtk::WeakReferenceWrapper<ToType>> Container;

  using iterator = typename Container::iterator;
  using const_iterator = typename Container::const_iterator;

  /// Construct an OrderedArcs instance from a node of type FromType to multiple
  /// nodes of type ToType.
  template<typename... ToTypes, typename = detail::CompatibleTypes<ToType, ToTypes...>>
  OrderedArcs(const FromType& from, ToTypes&... to)
    : m_from(from)
    , m_to({ smtk::weakRef(to)... })
  {
  }

  /// Construct an OrderedArcs instance from a node of type FromType to multiple
  /// nodes of type ToType, defined using begin and end iterators.
  template<typename Iterator>
  OrderedArcs(
    const FromType& from,
    typename std::enable_if<detail::is_iterable<Iterator>::type, const Iterator&>::type begin,
    const Iterator& end)
    : m_from(from)
  {
    m_to.reserve(std::distance(begin, end));
    for (Iterator it = begin; it != end; ++it)
    {
      m_to.push_back(smtk::weakRef(*it));
    }
  }

  /// Construct an OrderedArcs instance from a node of type FromType to multiple
  /// nodes of type ToType, defined by a container of ToTypes.
  template<typename Container>
  OrderedArcs(
    const FromType& from,
    typename std::enable_if<detail::is_container<Container>::type, const Container&>::type
      container)
    : OrderedArcs(from, container.begin(), container.end())
  {
  }

  const FromType& from() const { return m_from; }
  const Container& to() const { return m_to; }
  Container& to() { return m_to; }

  /// An API for accessing this class's information using
  /// smtk::graph::Component's API.
  template<typename SelfType>
  class API
  {
  protected:
    const SelfType& self(const FromType& lhs) const
    {
      return std::static_pointer_cast<smtk::graph::ResourceBase>(lhs.resource())
        ->arcs()
        .template get<SelfType>()
        .at(lhs.id());
    }

    SelfType& self(const FromType& lhs)
    {
      auto& arcs = std::static_pointer_cast<smtk::graph::ResourceBase>(lhs.resource())->arcs();
      if (!arcs.template contains<SelfType>(lhs.id()))
      {
        arcs.template emplace<SelfType>(lhs.id(), lhs);
      }
      return arcs.template get<SelfType>().at(lhs.id());
    }

  public:
    const Container& get(const FromType& lhs) const { return self(lhs).to(); }
    Container& get(const FromType& lhs) { return self(lhs).to(); }

    bool contains(const FromType& lhs) const
    {
      return std::static_pointer_cast<smtk::graph::ResourceBase>(lhs.resource())
        ->arcs()
        .template get<SelfType>()
        .contains(lhs.id());
    }
  };

  void removeExpired() const
  {
    using iterator = typename Container::iterator;
    Container* to_nodes = const_cast<Container*>(&m_to);
    iterator end = to_nodes->end();
    iterator new_end = to_nodes->begin();
    while (new_end != end && !new_end->expired())
    {
      ++new_end;
    }

    if (new_end == end)
    {
      return;
    }

    iterator walker = new_end;
    while (++walker != end)
    {
      if (!walker->expired())
      {
        *new_end = std::move(*walker);
        ++new_end;
      }
    }
    to_nodes->resize(std::distance(to_nodes->begin(), new_end));
  }

private:
  const FromType& m_from;
  Container m_to;
};
} // namespace graph
} // namespace smtk

#endif
