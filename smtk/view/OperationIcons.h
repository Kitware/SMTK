//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_OperationIcons_h
#define smtk_view_OperationIcons_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include "smtk/string/Token.h"

#include "smtk/common/Factory.h"

#include "smtk/operation/Operation.h" // for Operation::Index

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

namespace smtk
{
namespace view
{

/**\brief This factory class returns icon-constructors for Operations.
  *
  * An "icon" in this context is a binary array of the contents of an icon
  * representation given an input secondary color (typically the background
  * color). Currently, this corresponds to an ASCII description of an SVG image.
  *
  * An IconConstructor is a functor that accepts an Operation and a string
  * representing a secondary color and returns an icon for the Operation.
  * Because operations are hierarchical (i.e., they may inherit other operations),
  * IconConstructors registered to an operation may be used for derived
  * operations when those have no registered icon. In this case, it is possible
  * to use the provided Operation::Index to discern when an exact match or a
  * derived operation is passed to your IconConstructor functor and alter the
  * output accordindly.
  */
class SMTKCORE_EXPORT OperationIcons
{
public:
  /// A function that takes a secondary color and returns icon data (as a string).
  using IconConstructor = std::function<std::string(const std::string&)>;
  /// A function that takes a secondary color plus a mode returns icon data (as a string).
  using IconConstructor2 = std::function<std::string(const std::string&, smtk::string::Token)>;
  /// This class indexes icon-constructors the same way Operations are indexed.
  using Index = smtk::operation::Operation::Index;
  /// A map from operation names to operation indices used when creating icons by type-name.
  using IndexMap = std::unordered_map<std::string, Index>;
  /// A map from operation indices to construction functors used when creating icons.
  using FunctorMap = std::unordered_map<Index, IconConstructor>;
  /// A map from operation indices to improved construction functors used when creating icons.
  using FunctorMap2 = std::unordered_map<Index, IconConstructor2>;

  /// Register an icon constructor identified by the resource it represents.
  template<typename OperationType>
  bool registerOperation(IconConstructor&& functor)
  {
    Index index = typeid(OperationType).hash_code();
    bool didAdd = m_functors.emplace(std::make_pair(index, functor)).second;
    didAdd &=
      m_indices.emplace(std::make_pair(smtk::common::typeName<OperationType>(), index)).second;
    return didAdd;
  }

  template<typename OperationType>
  bool registerOperation(IconConstructor2&& functor)
  {
    Index index = typeid(OperationType).hash_code();
    bool didAdd = m_functors2.emplace(std::make_pair(index, functor)).second;
    didAdd &=
      m_indices.emplace(std::make_pair(smtk::common::typeName<OperationType>(), index)).second;
    return didAdd;
  }

  /// Register a default icon constructor. This constructor is used if no
  /// constructor can be found for an operation.
  void registerDefaultIconConstructor(IconConstructor&& functor);

  /// Unregister an icon identified by the resource it represents.
  template<typename OperationType>
  bool unregisterOperation()
  {
    Index index = typeid(OperationType).hash_code();
    for (auto it = m_indices.cbegin(); it != m_indices.cend(); /* do nothing */)
    {
      if (it->second == index)
      {
        it = m_indices.erase(it);
      }
      else
      {
        ++it;
      }
    }
    return m_functors.erase(index) > 0 || m_functors2.erase(index);
  }

  /// Construct an icon identified by the operation it represents.
  ///
  /// \a secondaryColor is a background or nearby color that the icon must contrast with.
  template<typename OperationType>
  std::string createIcon(const std::string& secondaryColor) const
  {
    using namespace smtk::string::literals;

    return this->createIcon<OperationType>(secondaryColor, "normal"_token);
  }

  /// Construct an icon identified by the operation it represents.
  ///
  /// \a secondaryColor is a background or nearby color that the icon must contrast with.
  /// \a mode is one of: "normal"_token, "disabled"_token, "selected"_token, "active"_token
  /// though this may change in the future.
  template<typename OperationType>
  std::string createIcon(const std::string& secondaryColor, smtk::string::Token mode) const
  {
    Index index = typeid(OperationType).hash_code();
    auto it = m_functors.find(index);
    auto it2 = m_functors2.find(index);
    if (it == m_functors.end() && it2 == m_functors2.end())
    {
      if (m_defaultIconConstructor)
      {
        return m_defaultIconConstructor(secondaryColor);
      }
      return std::string();
    }
    else if (it != m_functors.end())
    {
      return it->second(secondaryColor);
    }
    else
    {
      return it2->second(secondaryColor, mode);
    }
  }

  std::string createIcon(const std::string& operationName, const std::string& secondaryColor) const;
  std::string createIcon(
    const std::string& operationName,
    const std::string& secondaryColor,
    smtk::string::Token mode) const;
  std::string createIcon(const Index& index, const std::string& secondaryColor) const;
  std::string
  createIcon(const Index& index, const std::string& secondaryColor, smtk::string::Token mode) const;

private:
  FunctorMap m_functors;
  FunctorMap2 m_functors2;
  IndexMap m_indices;
  IconConstructor m_defaultIconConstructor;
};
} // namespace view
} // namespace smtk

#endif
