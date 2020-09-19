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
#include "smtk/resource/filter/Filter.h"

#include "tao/pegtl.hpp"
// PEGTL does not itself appear to do anything nasty, but
// on Windows MSVC 2015, it includes something that defines
// a macro named ERROR to be 0. This causes smtkErrorMacro()
// to expand into garbage (because smtk::io::Logger::ERROR
// gets expanded to smtk::io::Logger::0).
#ifdef ERROR
#undef ERROR
#endif

namespace smtk
{
namespace resource
{
namespace filter
{

using namespace tao::pegtl;

template <typename Type>
struct Name : plus<not_one<'\''> >
{
};

template <typename Type>
struct Regex : plus<not_one<'/'> >
{
};
}
}
}

#endif
