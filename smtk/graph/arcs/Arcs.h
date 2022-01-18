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
#include "smtk/graph/TypeTraits.h"

namespace smtk
{
namespace graph
{

/// An unordered collection of arcs of the same type.
///
/// All arcs must have components with the same origin and destination types.
template<typename from_type, typename to_type>
class SMTK_ALWAYS_EXPORT Arcs
{
public:
  typedef from_type FromType;
  typedef to_type ToType;
  typedef std::unordered_set<WeakReferenceWrapper<ToType>> Container;

  /// Construct an Arcs instance from a node of type FromType to multiple nodes
  /// of type ToType.
  template<typename... ToTypes, typename = CompatibleTypes<ToType, ToTypes...>>
  Arcs(const FromType& from, ToTypes&... to)
    : m_from(from)
    , m_to({ smtk::weakRef(to)... })
  {
  }

  /// Construct an Arcs instance from a node of type FromType to multiple nodes
  /// of type ToType, defined using begin and end iterators.
  template<typename Iterator>
  Arcs(
    const FromType& from,
    typename std::enable_if<is_iterable<Iterator>::type, const Iterator&>::type begin,
    const Iterator& end)
    : m_from(from)
  {
    m_to.reserve(std::distance(begin, end));
    for (Iterator it = begin; it != end; ++it)
    {
      m_to.insert(smtk::weakRef(*it));
    }
  }

  /// Construct an Arcs instance from a node of type FromType to multiple nodes
  /// of type ToType, defined by a container of ToTypes.
  template<typename Container>
  Arcs(
    const FromType& from,
    typename std::enable_if<is_container<Container>::type, const Container&>::type container)
    : Arcs(from, container.begin(), container.end())
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
