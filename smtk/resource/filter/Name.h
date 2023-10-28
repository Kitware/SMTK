//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_Name_h
#define smtk_resource_filter_Name_h

#include "smtk/resource/filter/Enclosed.h"

#include "tao/pegtl.hpp"

namespace smtk
{
namespace resource
{
namespace filter
{

using namespace tao::pegtl;

/// Match a persistent-object type-name (that is single-quoted).
template<typename Type>
struct Name : plus<not_one<'\''>>
{
};

/// Match a persistent-object type-name that is not quoted.
///
/// We continue until a square-bracket opening a property-clause appears.
/// This assumes that bare names are always followed by either EOF or a
/// property clause. If we ever want to match a succession of components
/// (say, by following arcs), this will need to change.
template<typename Type>
struct BareName : plus<not_one<'['>>
{
};

/// Match a regular expression for a persistent-object type-name
/// (which is "quoted" inside forward slashes).
template<typename Type>
struct Regex : plus<not_one<'/'>>
{
};
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
