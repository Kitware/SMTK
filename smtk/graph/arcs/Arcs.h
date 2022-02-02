//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_Arcs_h
#define smtk_graph_Arcs_h

#include <functional>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/WeakReferenceWrapper.h"

#include "smtk/graph/ResourceBase.h"
#include "smtk/graph/arcs/Inverse.h"
#include "smtk/graph/detail/TypeTraits.h"

namespace smtk
{
namespace graph
{

/// An unordered collection of arcs of the same type.
///
/// All arcs must have components with the same origin and destination types.
template<typename from_type, typename to_type, typename DerivedType = void>
class SMTK_ALWAYS_EXPORT Arcs
{
public:
  typedef from_type FromType;
  typedef to_type ToType;
  typedef std::unordered_set<WeakReferenceWrapper<ToType>> Container;
  typedef Inverse<DerivedType> InverseHandler;

  /// Construct an Arcs instance from a node of type FromType to multiple nodes
  /// of type ToType.
  template<typename... ToTypes, typename = detail::CompatibleTypes<ToType, ToTypes...>>
  Arcs(const FromType& from, ToTypes&... to)
    : m_from(from)
    , m_to({ smtk::weakRef(to)... })
  {
    for (auto& to : m_to)
    {
      if (!InverseHandler::insert(to, const_cast<FromType&>(m_from)))
      {
        throw std::logic_error("Failed to insert inverse during Arcs initialization.");
      }
    }
  }

  /// Construct an Arcs instance from a node of type FromType to multiple nodes
  /// of type ToType, defined using begin and end iterators.
  template<typename Iterator>
  Arcs(
    const FromType& from,
    typename std::enable_if<detail::is_iterable<Iterator>::type, const Iterator&>::type begin,
    const Iterator& end)
    : m_from(from)
  {
    this->insert(begin, end);
  }

  /// Construct an Arcs instance from a node of type FromType to multiple nodes
  /// of type ToType, defined by a container of ToTypes.
  template<typename IterableContainer>
  Arcs(
    const FromType& from,
    typename std::enable_if<
      detail::is_iterable_container<IterableContainer>::value,
      const IterableContainer&>::type container)
    : Arcs(from, container.begin(), container.end())
  {
  }

  template<typename IterableContainer>
  Arcs& operator=(typename std::enable_if<
                  detail::is_iterable_container<IterableContainer>::value,
                  const IterableContainer&>::type container)
  {
    this->clear();
    if (!this->insert(container.begin(), container.end()))
    {
      throw std::logic_error(
        "Failed to insert inverse during Arcs assignment from an iterable container.");
    }
  }

  Arcs& operator=(ToType& to)
  {
    this->clear();
    if (!this->insert(to).second)
    {
      throw std::logic_error(
        "Failed to insert inverse during Arcs assignment from an iterable container.");
    }
    return *this;
  }

  const FromType& from() const { return m_from; }
  const Container& to() const { return m_to; }
  Container& to() { return m_to; }

  typename Container::iterator begin() { return m_to.begin(); }
  typename Container::const_iterator begin() const { return m_to.begin(); }
  typename Container::iterator end() { return m_to.end(); }
  typename Container::const_iterator end() const { return m_to.end(); }

  std::size_t count() const { return m_to.size(); }

  template<typename IterableContainer>
  typename std::enable_if<detail::is_iterable_container<IterableContainer>::value, bool>::type
  insert(const IterableContainer& container, bool inverse = true)
  {
    return this->insert(container.begin(), container.end(), inverse);
  }

  template<typename Iterator>
  bool insert(const Iterator& first, const Iterator& last, bool inverse = true)
  {
    bool success = true;
    for (Iterator it = first; it != last; ++it)
    {
      // Success in a batch insertion is measured by if the inserted value exists in the
      // final container or not.
      success = !(this->insert(*it, inverse).first == m_to.end());
    }
    return success;
  }

  std::pair<typename Container::iterator, bool> insert(ToType& to, bool inverse = true)
  {
    auto result = m_to.insert(to);
    if (inverse && result.second)
    {
      if (!InverseHandler::insert(to, const_cast<FromType&>(m_from)))
      {
        m_to.erase(result.first);
        result.first = m_to.end();
        result.second = false;
      }
    }
    return result;
  }

  template<typename Iterator>
  std::size_t erase(const Iterator& first, const Iterator& last, bool inverse = true)
  {
    std::size_t numArcsRemoved = 0;
    for (Iterator it = first; it != last; ++it)
    {
      numArcsRemoved += this->erase(*it, inverse);
    }
    return numArcsRemoved;
  }

  std::size_t erase(const ToType& to, bool inverse = true)
  {
    std::size_t numArcsRemoved = 0;
    auto it = m_to.find(smtk::weakRef(to));
    if (it != m_to.end())
    {
      ++numArcsRemoved;
      m_to.erase(it);
      if (inverse)
      {
        numArcsRemoved += InverseHandler::erase(to, m_from);
      }
    }
    return numArcsRemoved;
  }

  void clear()
  {
    for (const auto& to : m_to)
    {
      if (to)
      {
        this->erase(to);
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
    Container& to_nodes = const_cast<Container&>(m_to);
    for (auto it = to_nodes.begin(); it != to_nodes.end();)
    {
      if (it->expired())
      {
        it = to_nodes.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

private:
  const FromType& m_from;
  Container m_to;
};
} // namespace graph
} // namespace smtk

#endif
