//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_categories_Actions_h
#define smtk_attribute_categories_Actions_h

#include "smtk/attribute/categories/Evaluators.h"
#include "smtk/attribute/categories/Grammar.h"

#include "smtk/common/TypeName.h"

// clang-format off

namespace smtk
{
namespace attribute
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
  static void apply(const Input&, smtk::attribute::categories::Evaluators& evals)
  {
    evals.startSubExpression();
  }
};

///\brief Processing the end of the current sub-expression
template<>
struct Action<one< ')' >>
{
  template<typename Input>
  static void apply(const Input&, smtk::attribute::categories::Evaluators& evals)
  {
    evals.endSubExpression();
  }
};

///\brief Generate lambda for matching a category name defined within single quotes
template<>
struct Action<smtk::attribute::categories::NameSyntax>
{
  template<typename Input>
  static void apply(const Input& input, smtk::attribute::categories::Evaluators& evals)
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
struct Action<smtk::attribute::categories::BareNameSyntax>
{
  template<typename Input>
  static void apply(const Input& input, smtk::attribute::categories::Evaluators& evals)
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
struct Action<smtk::attribute::categories::ComplementOperator>
{
  template<typename Input>
  static void apply(const Input&, smtk::attribute::categories::Evaluators& evals)
  {
    evals.pushOp('!');
  }
};

///\brief Push an and symbol onto the operation stack
template<>
struct Action<smtk::attribute::categories::AndOperator>
{
  template<typename Input>
  static void apply(const Input&, smtk::attribute::categories::Evaluators& evals)
  {
    evals.pushOp('&');
  }
};

///\brief Push an or symbol onto the operation stack
template<>
struct Action<smtk::attribute::categories::OrOperator>
{
  template<typename Input>
  static void apply(const Input&, smtk::attribute::categories::Evaluators& evals)
  {
    evals.pushOp('|');
  }
};

// clang-format on

} // namespace categories
} // namespace attribute
} // namespace smtk

#endif
