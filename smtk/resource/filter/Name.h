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

template<typename Type>
struct Name : plus<not_one<'\''>>
{
};

template<typename Type>
struct Regex : plus<not_one<'/'>>
{
};
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
