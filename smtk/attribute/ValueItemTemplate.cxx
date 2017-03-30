//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/ValueItemTemplate.h"

namespace smtk
{
namespace attribute
{
// To auto-generate pybind11 bindings, we read out class information using
// pygccxml, which only reports template instances. We therefore explicitly
// declare the three used instances of ValueItemTemplate to facilitate the
// scripted generation of pybind11 bindings.
template class ValueItemTemplate<int>;
template class ValueItemTemplate<double>;
template class ValueItemTemplate<std::string>;
}
}
