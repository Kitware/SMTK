//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_Grammar_h
#define smtk_resource_filter_Grammar_h

#include "smtk/CoreExports.h"

#include "smtk/resource/filter/FloatingPointGrammar.h"
#include "smtk/resource/filter/IntegerGrammar.h"
#include "smtk/resource/filter/StringGrammar.h"
#include "smtk/resource/filter/VectorGrammar.h"

namespace smtk
{
namespace resource
{
namespace filter
{

/// SMTK's default grammar for filtering Components relies on the Components'
/// properties (see `smtk::resource::PersistentObject::properties()`). Derived
/// resources can redefine this grammar, adding or removing syntax elements and
/// augmenting the list of parsable property types as needed.
struct SMTKCORE_EXPORT Grammar
  : bracketed<sor<
      Property<long>::Grammar,
      Property<double>::Grammar,
      Property<std::string>::Grammar,
      Property<std::vector<long>>::Grammar,
      Property<std::vector<double>>::Grammar,
      Property<std::vector<std::string>>::Grammar>>
{
};
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
