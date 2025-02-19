//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_categories_Actions_h
#define smtk_common_categories_Actions_h

#include "smtk/common/categories/Evaluators.h"
#include "smtk/common/categories/Grammar.h"

#include "smtk/common/TypeName.h"

// clang-format off

namespace smtk
{
namespace common
{
namespace categories
{

template<typename Clause>
struct Action : nothing<Clause>
{
};

///\brief Processing the start of a new sub-expression
template<>
struct Action<one< '(' >>
{
  template<typename Input>
  static void apply(const Input&, smtk::common::categories::Evaluators& evals)
  {
    evals.startSubExpression();
  }
};

///\brief Processing the end of the current sub-expression
template<>
struct Action<one< ')' >>
{
  template<typename Input>
  static void apply(const Input&, smtk::common::categories::Evaluators& evals)
  {
    evals.endSubExpression();
  }
};

///\brief Generate lambda for matching a category name defined within single quotes
template<>
struct Action<smtk::common::categories::NameSyntax>
{
  template<typename Input>
  static void apply(const Input& input, smtk::common::categories::Evaluators& evals)
  {
    std::string catName = input.string();
    evals.addCategoryName(catName);
    evals.pushEval([catName](const std::set<std::string>& catNames){
    return (catNames.find(catName) != catNames.end());
    });
  }
};

///\brief Generate lambda for matching a category name defined without using quotes
template<>
struct Action<smtk::common::categories::BareNameSyntax>
{
  template<typename Input>
  static void apply(const Input& input, smtk::common::categories::Evaluators& evals)
  {
    std::string catName = input.string();
    evals.addCategoryName(catName);
    evals.pushEval([catName](const std::set<std::string>& catNames){
    return (catNames.find(catName) != catNames.end());
    });
  }
};

///\brief Push a complement symbol onto the operation stack
template<>
struct Action<smtk::common::categories::ComplementOperator>
{
  template<typename Input>
  static void apply(const Input&, smtk::common::categories::Evaluators& evals)
  {
    evals.pushOp('!');
  }
};

///\brief Push an and symbol onto the operation stack
template<>
struct Action<smtk::common::categories::AndOperator>
{
  template<typename Input>
  static void apply(const Input&, smtk::common::categories::Evaluators& evals)
  {
    evals.pushOp('&');
  }
};

///\brief Push an or symbol onto the operation stack
template<>
struct Action<smtk::common::categories::OrOperator>
{
  template<typename Input>
  static void apply(const Input&, smtk::common::categories::Evaluators& evals)
  {
    evals.pushOp('|');
  }
};

// clang-format on

} // namespace categories
} // namespace common
} // namespace smtk

#endif
