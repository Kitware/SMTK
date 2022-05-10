//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_NodeProperties_h
#define smtk_graph_NodeProperties_h

#include "smtk/Metaprogramming.h"
#include "smtk/common/Visit.h"
#include "smtk/string/Token.h"

#include <functional>
#include <iterator>
#include <set>
#include <type_traits>

namespace smtk
{
namespace graph
{

/**\brief Checks that can be performed on a node-type.
  *
  * Note that the properties defined in this class are named like so:
  * + direct checks on the \a NodeType template parameter are named `hasXXX`.
  * + results that specify behavior of the resulting node implementation are named `isXXX`.
  * (i.e., "If the traits have this, then the node is that.")
  */
template<typename NodeType>
class NodeProperties
{
  template<class>
  struct type_sink
  {
    using type = void;
  }; // consume a type and make it `void`
  template<class T>
  using type_sink_t = typename type_sink<T>::type;

public:
  /**\brief Check whether the node type is marked to be serialized/deserialized.
    *
    * The default is **not** to serialize node types.
    */
  template<class T, class = void>
  struct hasSerializeMark : std::false_type
  {
  };
  template<class T>
  struct hasSerializeMark<T, type_sink_t<typename T::Serialize>>
    : std::conditional<T::Serialize::value, std::true_type, std::false_type>::type
  {
  };

  /// True when a node type should be serialized.
  class isSerializable
  {
  public:
    using type = typename hasSerializeMark<NodeType>::type;
    static constexpr bool value = type::value;
  };
};

} // namespace graph
} // namespace smtk

#endif
