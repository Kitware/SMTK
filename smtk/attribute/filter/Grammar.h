//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_filter_Grammar_h
#define smtk_attribute_filter_Grammar_h

#include "smtk/CoreExports.h"

#include "smtk/attribute/filter/Attribute.h"

#include "smtk/resource/filter/FloatingPoint.h"
#include "smtk/resource/filter/Integer.h"
#include "smtk/resource/filter/String.h"

namespace smtk
{
namespace attribute
{
namespace filter
{

/// SMTK's grammar for filtering Attributes relies on the Attribute's
/// properties (see `smtk::resource::PersistentObject::properties()`) as well as the Attribute's
/// Definition type information.
struct SMTKCORE_EXPORT Grammar
  : seq<
      sor<TAO_PEGTL_ISTRING("attribute"), TAO_PEGTL_STRING("*"), TAO_PEGTL_ISTRING("any")>,
      opt<smtk::resource::filter::bracketed<list_must<
        pad<
          sor<
            smtk::resource::filter::Property<long>::Grammar,
            smtk::resource::filter::Property<double>::Grammar,
            smtk::resource::filter::Property<std::string>::Grammar,
            smtk::resource::filter::Property<std::vector<long>>::Grammar,
            smtk::resource::filter::Property<std::vector<double>>::Grammar,
            smtk::resource::filter::Property<std::vector<std::string>>::Grammar,
            AttributeTypeSpec::Grammar>,
          space>,
        TAO_PEGTL_ISTRING(",")>>>>
{
};
} // namespace filter
} // namespace attribute
} // namespace smtk

#endif
