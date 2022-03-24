//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/*! \file */
#ifndef smtk_common_Visit_h
#define smtk_common_Visit_h

#include <type_traits>
#include <utility>

namespace smtk
{
namespace common
{

/// Return values common to most visitor methods.
enum class Visit
{
  Continue, //!< Continue to visit items.
  Halt      //!< Stop visiting items immediately.
};

/**\brief Return value for functions/methods that accept visitors.
  *
  * If your function or class-method accepts functors that
  * may return Visit::Halt, then you should return an enumerant
  * below to indicate whether iteration terminated early or not.
  */
enum class Visited
{
  All,  //!< The visitor was invoked on every item exhaustively.
  Some, //!< A visitor signaled early termination.
  Empty //!< The were no values to visit.
};

/**\brief A template for accepting visitors with different return types.
  *
  * Functions or class-methods that accept visitor functors used to
  * iterate over a set of values may wish to allow visitors to terminate
  * iteration early. However, in order to do so, visitors must return
  * some value to indicate whether to continue or halt.
  *
  * This templated functor class can be used to adapt functors that
  * return no value to exhaustively iterate while also accepting
  * functors that do provide cues on when to stop prematurely.
  *
  * See UnitTestVisit in the testing subdirectory for a simple example.
  */
template<typename Functor>
struct VisitorFunctor
{
  VisitorFunctor(Functor f)
    : m_functor(f)
  {
  }

  /// For functors that return a Visit enumerant, simply invoke them:
  template<class... Types>
  auto invokeVisitor(Types&&... args) const -> typename std::
    enable_if<std::is_same<decltype(std::declval<Functor>()(args...)), Visit>::value, Visit>::type
  {
    return m_functor(std::forward<Types>(args)...);
  }

  /// For functors that do not return a Visit enumerant (i.e., void),
  /// invoke them but always return Visit::Continue.
  template<class... Types>
  auto invokeVisitor(Types&&... args) const -> typename std::
    enable_if<!std::is_same<decltype(std::declval<Functor>()(args...)), Visit>::value, Visit>::type
  {
    m_functor(std::forward<Types>(args)...);
    return Visit::Continue;
  }

  /// The parenthesis operation simply forwards arguments to whichever method above is enabled.
  template<class... Types>
  Visit operator()(Types&&... args) const
  {
    return this->invokeVisitor(std::forward<Types>(args)...);
  }

  Functor m_functor;
};

} // namespace common
} // namespace smtk

#endif // smtk_common_Visit_h
