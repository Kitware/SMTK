//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_filter_GrammarInfoActions_h
#define smtk_attribute_filter_GrammarInfoActions_h

#include "smtk/attribute/filter/Grammar.h"
#include "smtk/attribute/filter/GrammarInfo.h"

#include "smtk/resource/filter/FloatingPointGrammar.h"
#include "smtk/resource/filter/IntegerGrammar.h"
#include "smtk/resource/filter/StringGrammar.h"
#include "smtk/resource/filter/VectorGrammar.h"

///\file Actions used to determine the information stored in an
/// attribute query string.
///
/// Currently the information extracted include:
/// - The definition type as well as an indication if this represents
///   a regular expression
/// - An indication that the query contains property constraints

namespace smtk
{
namespace attribute
{
namespace filter
{

struct GrammarInfoAttributeTypeSpecTypeNameAction
{
  template<typename Input>
  static void apply(const Input& input, GrammarInfo& info)
  {
    info.setTypeInfo(input.string());
  }
};

struct GrammarInfoAttributeTypeSpecTypeRegexAction
{
  template<typename Input>
  static void apply(const Input& input, GrammarInfo& info)
  {
    info.setTypeInfo(input.string(), true);
  }
};

struct GrammarInfoPropertyAction
{
  template<typename Input>
  static void apply(const Input& input, GrammarInfo& info)
  {
    (void)input;
    info.setHasProperties();
  }
};

template<typename Info>
struct ExtractGrammarAction : nothing<Info>
{
};

template<>
struct ExtractGrammarAction<AttributeTypeSpec::TypeName>
  : GrammarInfoAttributeTypeSpecTypeNameAction
{
};
template<>
struct ExtractGrammarAction<AttributeTypeSpec::TypeRegex>
  : GrammarInfoAttributeTypeSpecTypeRegexAction
{
};

template<>
struct ExtractGrammarAction<typename smtk::resource::filter::Property<double>::Grammar>
  : GrammarInfoPropertyAction
{
};
template<>
struct ExtractGrammarAction<typename smtk::resource::filter::Property<std::vector<double>>::Grammar>
  : GrammarInfoPropertyAction
{
};
template<>
struct ExtractGrammarAction<typename smtk::resource::filter::Property<long>::Grammar>
  : GrammarInfoPropertyAction
{
};
template<>
struct ExtractGrammarAction<typename smtk::resource::filter::Property<std::vector<long>>::Grammar>
  : GrammarInfoPropertyAction
{
};
template<>
struct ExtractGrammarAction<typename smtk::resource::filter::Property<std::string>::Grammar>
  : GrammarInfoPropertyAction
{
};
template<>
struct ExtractGrammarAction<
  typename smtk::resource::filter::Property<std::vector<std::string>>::Grammar>
  : GrammarInfoPropertyAction
{
};

} // namespace filter
} // namespace attribute
} // namespace smtk

#endif
