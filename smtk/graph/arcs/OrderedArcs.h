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

#include <algorithm>
#include <functional>
#include <type_traits>
#include <vector>

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/WeakReferenceWrapper.h"

#include "smtk/graph/arcs/Inverse.h"
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
template<typename from_type, typename to_type, typename DerivedType = void>
class SMTK_ALWAYS_EXPORT OrderedArcs
{
public:
  typedef from_type FromType;
  typedef to_type ToType;
  typedef std::vector<smtk::WeakReferenceWrapper<ToType>> Container;
  typedef Inverse<DerivedType> InverseHandler;

  using iterator = typename Container::iterator;
  using const_iterator = typename Container::const_iterator;

  /// Construct an OrderedArcs instance from a node of type FromType to multiple
  /// nodes of type ToType.
  template<typename... ToTypes, typename = detail::CompatibleTypes<ToType, ToTypes...>>
  OrderedArcs(const FromType& from, ToTypes&... to)
    : m_from(from)
    , m_to({ smtk::weakRef<ToType>(to)... })
  {
    for (const auto& to : m_to)
    {
      if (!InverseHandler::insert(to, const_cast<FromType&>(m_from)))
      {
        throw std::logic_error("Failed to insert inverse during OrderedArcs initialized a list");
      }
    }
  }

  /// Construct an OrderedArcs instance from a node of type FromType to multiple
  /// nodes of type ToType, defined using begin and end iterators.
  template<typename Iterator>
  OrderedArcs(
    const FromType& from,
    typename std::enable_if<detail::is_forward_iterable<Iterator>::value, const Iterator&>::type
      begin,
    const Iterator& end)
    : m_from(from)
  {
    if (!(this->insert(begin, end)))
    {
      throw std::logic_error(
        "Failed to insert inverse during OrderedArcs initialized a iterator range");
    }
    m_to.shrink_to_fit();
  }

  /// Construct an OrderedArcs instance from a node of type FromType to multiple
  /// nodes of type ToType, defined by a container of ToTypes.
  template<typename IterableContainer>
  OrderedArcs(
    const FromType& from,
    typename std::enable_if<
      detail::is_iterable_container<IterableContainer>::value,
      const IterableContainer&>::type container)
    : OrderedArcs(from, container.begin(), container.end())
  {
  }

  const FromType& from() const { return m_from; }
  const Container& to() const { return m_to; }
  Container& to() { return m_to; }

  typename Container::iterator begin() { return m_to.begin(); }
  typename Container::const_iterator begin() const { return m_to.begin(); }
  typename Container::iterator end() { return m_to.end(); }
  typename Container::const_iterator end() const { return m_to.end(); }

  std::size_t count() const { return m_to.size(); }

  // NOLINTBEGIN(misc*)
  template<typename IterableContainer>
  typename std::enable_if<detail::is_iterable_container<IterableContainer>::value, OrderedArcs&>::
    type
    operator=(const IterableContainer& to)
  {
    this->clear();
    if (!(this->insert(to.begin(), to.end())))
    {
      throw std::logic_error(
        "Failed to insert inverse during OrderedArcs assignment from an iterable container.");
    }
    return *this;
  }
  // NOLINTEND(misc*)

  OrderedArcs& operator=(ToType& to)
  {
    this->clear();
    if (!(this->insert_front(to).second))
    {
      throw std::logic_error(
        "Failed to insert inverse during OrderedArcs assignment from a single value.");
    }
    return *this;
  }

  /// Add the arc from->to
  template<typename IterableContainer>
  typename std::enable_if<detail::is_iterable_container<IterableContainer>::value, bool>::type
  insert(const IterableContainer& container)
  {
    return this->insert(container.begin(), container.end());
  }

  template<typename Iterator>
  typename std::enable_if<detail::is_forward_iterable<Iterator>::value, bool>::type insert(
    const Iterator& first,
    const Iterator& last)
  {
    bool success = true;
    for (Iterator it = first; it != last; ++it)
    {
      success = (success && this->insert_back(*it).second);
    }
    return success;
  }

  std::pair<typename Container::iterator, bool>
  insert(typename Container::iterator pos, ToType& to, bool inverse = true)
  {
    bool doinsert = true;
    std::pair<typename Container::iterator, bool> result(m_to.end(), false);
    if (inverse)
    {
      doinsert = InverseHandler::insert(to, const_cast<FromType&>(m_from));
    }
    if (doinsert)
    {
      result.first = m_to.insert(pos, smtk::weakRef(to));
    }
    result.second = doinsert;
    return result;
  }

  std::pair<typename Container::iterator, bool> insert_front(ToType& to, bool inverse = true)
  {
    return this->insert(m_to.begin(), to, inverse);
  }

  std::pair<typename Container::iterator, bool> insert_back(ToType& to, bool inverse = true)
  {
    return this->insert(m_to.end(), to, inverse);
  }

  /// Remove the first match of from->to
  std::size_t erase(const ToType& to, bool inverse = true)
  {
    std::size_t numArcsErased = 0;
    auto it = std::find(m_to.begin(), m_to.end(), smtk::weakRef(to));
    if (it != m_to.end())
    {
      ++numArcsErased;
      m_to.erase(it);
      if (inverse)
      {
        numArcsErased += InverseHandler::erase(to, m_from);
      }
    }
    return numArcsErased;
  }

  void clear()
  {
    for (auto& to : m_to)
    {
      if (to)
      {
        InverseHandler::erase(to, m_from);
      }
    }
    m_to.clear();
  }

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
    const SelfType& get(const FromType& lhs) const { return self(lhs); }
    SelfType& get(const FromType& lhs) { return self(lhs); }

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

protected:
  template<typename, typename>
  friend class Inverse;
  /// Needed to break loop in InverseHandler, never inserts
  std::pair<typename Container::iterator, bool> insert(ToType&, bool = true)
  {
    std::pair<typename Container::iterator, bool> result(m_to.end(), false);
    return result;
  }

private:
  const FromType& m_from;
  Container m_to;
};

} // namespace graph
} // namespace smtk

#endif
