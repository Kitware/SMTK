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

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/TypeContainer.h"
#include "smtk/graph/ArcImplementation.h"

namespace smtk
{
namespace graph
{

/**\brief A container for arcs held by a resource.
  *
  * This class extends TypeContainer in two ways:
  * + its constructors decorate the arc type-traits parameters passed to it with
  *   API implementations for components to use.
  * + it deletes the copy/assignment constructors so developers must reference
  *   the container instead of mistakenly modifying an accidental copy.
  */
class SMTKCORE_EXPORT ArcMap : public smtk::common::TypeContainer
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

  ~ArcMap() override = default;

  /**\brief Return the arc's implementation object given its trait type.
    *
    * This will return null when the arc map does not hold arcs of the
    * given trait-type.
    */
  //@{
  template<typename ArcTraits>
  const ArcImplementation<ArcTraits>* at() const
  {
    if (this->contains<ArcImplementation<ArcTraits>>())
    {
      const auto& arcObject = this->get<ArcImplementation<ArcTraits>>();
      return &arcObject;
    }
    return nullptr;
  }

  template<typename ArcTraits>
  ArcImplementation<ArcTraits>* at()
  {
    if (this->contains<ArcImplementation<ArcTraits>>())
    {
      auto& arcObject = this->get<ArcImplementation<ArcTraits>>();
      return &arcObject;
    }
    return nullptr;
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

protected:
  template<typename Type>
  void insertArcImplementation()
  {
    ArcImplementation<Type> arcObject;
    this->insert(arcObject);
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
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_ArcMap_h
