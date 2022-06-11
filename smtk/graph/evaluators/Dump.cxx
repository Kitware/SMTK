//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/graph/evaluators/Dump.h"

namespace smtk
{
namespace graph
{
namespace evaluators
{

std::unique_ptr<std::array<double, 4>> Dump::s_backgroundColor;

} // namespace evaluators
} // namespace graph
} // namespace smtk
