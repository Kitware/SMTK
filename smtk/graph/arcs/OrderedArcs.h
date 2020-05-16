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

#include "smtk/graph/arcs/Extensions.h"

namespace smtk
{
namespace graph
{

/// An ordered collection of arcs of the same type.
///
/// All arcs must have components with the same origin and destination types.
/// Furthermore, the arcs are ordered so that they are reported or visited in a
/// consistent, user-specified order.
template <typename from_type, typename to_type>
class SMTK_ALWAYS_EXPORT OrderedArcs
{
public:
  typedef from_type FromType;
  typedef to_type ToType;
  typedef std::vector<std::reference_wrapper<const ToType> > Container;

  /// Construct an OrderedArcs instance from a node of type FromType to multiple
  /// nodes of type ToType.
  template <typename... ToTypes, typename = CompatibleTypes<ToType, ToTypes...> >
  OrderedArcs(const FromType& from, ToTypes const&... to)
    : m_from(from)
    , m_to({ std::ref(to)... })
  {
  }

  /// Construct an OrderedArcs instance from a node of type FromType to multiple
  /// nodes of type ToType, defined using begin and end iterators.
  template <typename Iterator>
  OrderedArcs(const FromType& from,
    typename std::enable_if<is_iterable<Iterator>::type, const Iterator&>::type begin,
    const Iterator& end)
    : m_from(from)
  {
    m_to.reserve(std::distance(begin, end));
    for (Iterator it = begin; it != end; ++it)
    {
      m_to.push_back(std::ref(*it));
    }
  }

  /// Construct an OrderedArcs instance from a node of type FromType to multiple
  /// nodes of type ToType, defined by a container of ToTypes.
  template <typename Container>
  OrderedArcs(const FromType& from,
    typename std::enable_if<is_container<Container>::type, const Container&>::type container)
    : OrderedArcs(from, container.begin(), container.end())
  {
  }

  const FromType& from() const { return m_from; }
  const Container& to() const { return m_to; }
  Container& to() { return m_to; }

  /// An API for accessing this class's information using
  /// smtk::graph::Component's API.
  template <typename SelfType>
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

private:
  const FromType& m_from;
  std::vector<std::reference_wrapper<const ToType> > m_to;
};
}
}

#endif
