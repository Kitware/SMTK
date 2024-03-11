//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_ArcMap_h
#define smtk_graph_ArcMap_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/graph/ArcImplementation.h"
#include "smtk/graph/Directionality.h"

#include "smtk/resource/query/BadTypeError.h"

#include "smtk/string/Token.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/RuntimeTypeContainer.h"
#include "smtk/common/TypeName.h"

#include <set>

namespace smtk
{
namespace resource
{
class CopyOptions;
}
namespace graph
{

class ResourceBase;

/**\brief A container for arcs held by a resource.
  *
  * This class extends RuntimeTypeContainer in two ways:
  * + its constructors decorate the arc type-traits parameters passed to it with
  *   API implementations for components to use.
  * + it deletes the copy/assignment constructors so developers must reference
  *   the container instead of mistakenly modifying an accidental copy.
  */
class SMTKCORE_EXPORT ArcMap
{
public:
  smtkTypeMacroBase(smtk::graph::ArcMap);
  smtkSuperclassMacro(smtk::common::TypeMap<smtk::common::UUID>);
  using key_type = smtk::common::UUID;

  ArcMap() {} // NOLINT . MSVC2019 barfs with C2580 on "= default;"

  template<typename List>
  ArcMap()
  {
    this->insertArcImplementations<List>();
  }

  template<typename List>
  ArcMap(identity<List>)
  {
    this->insertArcImplementations<List>();
  }

  /// Do not allow the map to be copied:
  ArcMap(const ArcMap&) = delete;
  ArcMap& operator=(const ArcMap&) = delete;

  virtual ~ArcMap() = default;

  /**\brief Return the arc's implementation object given its trait type.
    *
    * This will return null when the arc map does not hold arcs of the
    * given trait-type.
    */
  //@{
  template<typename ArcTraits>
  const ArcImplementation<ArcTraits>* at() const
  {
    smtk::string::Token arcTypeName = smtk::common::typeName<ArcTraits>();
    auto it = m_data.find(arcTypeName);
    if (it == m_data.end())
    {
      return nullptr;
    }
    return dynamic_cast<ArcImplementation<ArcTraits>*>(it->second.get());
  }

  template<typename ArcTraits>
  ArcImplementation<ArcTraits>* at()
  {
    smtk::string::Token arcTypeName = smtk::common::typeName<ArcTraits>();
    auto it = m_data.find(arcTypeName);
    if (it == m_data.end())
    {
      return nullptr;
    }
    return dynamic_cast<ArcImplementation<ArcTraits>*>(it->second.get());
  }

  template<typename ImplementationType>
  const ImplementationType* at(smtk::string::Token declaredType) const
  {
    auto it = m_data.find(declaredType);
    if (it == m_data.end())
    {
      return nullptr;
    }
    return dynamic_cast<ImplementationType*>(it->second.get());
  }

  template<typename ImplementationType>
  ImplementationType* at(smtk::string::Token declaredType)
  {
    auto it = m_data.find(declaredType);
    if (it == m_data.end())
    {
      return nullptr;
    }
    return dynamic_cast<ImplementationType*>(it->second.get());
  }
  //@}

  /**\brief Invoke a \a Functor (which accepts \a args) on each arc type in the \a Tuple.
    *
    * For each arc type, the functor is passed the input arguments (which it may
    * modify). Note that the object passed to each functor will be an
    * ArcImplementation<ArcTraits>, where each ArcTraits is drawn from the \a Tuple.
    */
  //@{
  template<typename Tuple, typename Functor, typename... Args>
  void invoke(Args&&... args) const
  {
    ArcMap::invokeFunctors<0, Tuple, Functor>(std::forward<Args>(args)...);
  }

  template<typename Tuple, typename Functor, typename... Args>
  void invoke(Args&&... args)
  {
    ArcMap::invokeFunctors<0, Tuple, Functor>(std::forward<Args>(args)...);
  }
  //@}

  /**\brief Invoke a \a Functor (which accepts \a args) on each run-time arc type in the \a Tuple.
    *
    * For each RuntimeArc instance, the functor is passed the input arguments (which it may
    * modify). Note that the object passed to each functor will be an
    * ArcImplementationBase (a pure virtual base class for ArcImplementation<T>).
    * This is less efficient than the compile-time arc support, but is more flexible.
    */
  //@{
  // const version
  template<typename Functor, typename... Args>
  void invokeRuntime(Args&&... args) const
  {
    Functor f;
    for (const auto& baseArc : this->runtimeBaseTypes())
    {
      for (const auto& arcTypeName : this->runtimeTypeNames(baseArc))
      {
        f(arcTypeName, *this->at<ArcImplementationBase>(arcTypeName), std::forward<Args>(args)...);
      }
    }
  }

  // non-const version
  template<typename Functor, typename... Args>
  void invokeRuntime(Args&&... args)
  {
    Functor f;
    for (const auto& baseArc : this->runtimeBaseTypes())
    {
      for (const auto& arcTypeName : this->runtimeTypeNames(baseArc))
      {
        f(arcTypeName, *this->at<ArcImplementationBase>(arcTypeName), std::forward<Args>(args)...);
      }
    }
  }
  //@}

  /// Return the type-names of the arc types accepted by this ArcMap instance.
  const std::set<smtk::string::Token>& types() const { return m_types; }

  /// Insert a run-time arc type.
  ///
  /// The \a arcType will be used as a "declared typename".
  /// If \a directed is true, arcs will be directed.
  ///
  /// The \a fromNodeSpecs and \a toNodeSpecs are sets of
  /// [query-filters](https://smtk.readthedocs.io/en/latest/userguide/resource/filtering-and-searching.html)
  /// that specify components within the resource which may serve
  /// as the arc's endpoints. Note that if \a directed is false,
  /// then the union of both node specs is used but a warning will
  /// be emitted if they are not identical.
  ///
  /// This method will return false if your graph-resource's type traits
  /// object does not include the RuntimeArc in its ArcTypes tuple.
  ///
  /// This method requires a \a resource in order to call queryOperation()
  /// on the endpoint type-specifiers. (Note that each graph resource's
  /// traits object may have a different set of node and arc types which
  /// affect the grammar of type-specifier strings.)
  bool insertRuntimeArcType(
    smtk::graph::ResourceBase* resource,
    smtk::string::Token arcType,
    std::unordered_set<smtk::string::Token> fromNodeSpecs,
    std::unordered_set<smtk::string::Token> toNodeSpecs,
    Directionality isDirected);

  /// Return the set of object-types that hold run-time arcs.
  ///
  /// Each object-type may have multiple corresponding run-time arcs.
  /// \sa runtimeTypeNames
  std::unordered_set<smtk::string::Token> runtimeBaseTypes() const;

  /// Return the set of run-time arcs given a base object-type.
  ///
  /// \sa runtimeBaseTypes
  const std::unordered_set<smtk::string::Token>& runtimeTypeNames(
    smtk::string::Token baseType) const;

  /// Return a reference to an arc-type's storage.
  template<typename ArcImplementationType>
  const ArcImplementationType& getRuntime(smtk::string::Token arcType) const
  {
    auto it = m_data.find(arcType);
    if (it == m_data.end())
    {
      throw smtk::resource::query::BadTypeError(arcType.data());
    }
    return *dynamic_cast<ArcImplementationType*>(it->second.get());
  }

  template<typename ArcImplementationType>
  ArcImplementationType& getRuntime(smtk::string::Token arcType)
  {
    auto it = m_data.find(arcType);
    if (it == m_data.end())
    {
      throw smtk::resource::query::BadTypeError(arcType.data());
    }
    return *dynamic_cast<ArcImplementationType*>(it->second.get());
  }

  /// Copy arcs from a \a source resource's arcs.
  ///
  /// This method is only intended for use from within graph-resource
  /// overrides of smtk::resource::Resource::copyRelations(), where
  /// the \a source is a resource sharing the same set of compile-time
  /// traits.
  ///
  /// This method copies run-time arc types and then copies arcs
  /// themselves, translating endpoint component UUIDs via the \a options
  /// provided.
  ///
  /// Only explicit arcs are copied.
  virtual void copyArcs(
    const smtk::graph::ResourceBase* source,
    //const ArcMap& source,
    smtk::resource::CopyOptions& options,
    smtk::graph::ResourceBase* target);

protected:
  template<typename Type>
  void insertArcImplementation()
  {
    smtk::string::Token arcTypeName = smtk::common::typeName<Type>();
    auto arcData = std::make_shared<ArcImplementation<Type>>();
    if (m_data.emplace(arcTypeName, arcData).second)
    {
      m_types.insert(arcTypeName);
    }
  }

  // const version
  template<typename Entry, typename Functor, typename... Args>
  void invokeFunctor(Args&&... args) const
  {
    Functor f;
    f(this->at<Entry>(), std::forward<Args>(args)...);
  }

  // non-const version
  template<typename Entry, typename Functor, typename... Args>
  void invokeFunctor(Args&&... args)
  {
    Functor f;
    f(this->at<Entry>(), std::forward<Args>(args)...);
  }

  template<typename Tuple>
  void insertArcImplementations()
  {
    ArcMap::insertArcImplementations<0, Tuple>();
  }

private:
  template<std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value>::type
  insertArcImplementations()
  {
    this->insertArcImplementation<typename std::tuple_element<I, Tuple>::type>();
    ArcMap::insertArcImplementations<I + 1, Tuple>();
  }

  template<std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value>::type
  insertArcImplementations()
  {
  }

  // const versions
  template<std::size_t I, typename Tuple, typename Functor, typename... Args>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value>::type invokeFunctors(
    Args&&... args) const
  {
    this->invokeFunctor<typename std::tuple_element<I, Tuple>::type, Functor>(
      std::forward<Args>(args)...);
    ArcMap::invokeFunctors<I + 1, Tuple, Functor>(std::forward<Args>(args)...);
  }

  // non-const version
  template<std::size_t I, typename Tuple, typename Functor, typename... Args>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value>::type invokeFunctors(
    Args&&... args)
  {
    this->invokeFunctor<typename std::tuple_element<I, Tuple>::type, Functor>(
      std::forward<Args>(args)...);
    ArcMap::invokeFunctors<I + 1, Tuple, Functor>(std::forward<Args>(args)...);
  }

  // This only needs a const version.
  template<std::size_t I, typename Tuple, typename Functor, typename... Args>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value>::type invokeFunctors(
    Args&&...) const
  {
  }

  /// Map from type (or declared type for runtime arcs) to a base implementation class.
  std::unordered_map<smtk::string::Token, std::shared_ptr<ArcImplementationBase>> m_data;
  /// A map from base runtime type (e.g., RuntimeArc<IsDirected>, RuntimeArc<IsUndirected>) to
  /// the declared type of runtime arcs using the base runtime type.
  std::unordered_map<smtk::string::Token, std::unordered_set<smtk::string::Token>> m_runtimeArcs;
  /// A cache of the keys of m_data.
  std::set<smtk::string::Token> m_types;
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_ArcMap_h
