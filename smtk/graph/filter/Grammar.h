//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_filter_Grammar_h
#define smtk_graph_filter_Grammar_h

#include "smtk/CoreExports.h"

#include "smtk/graph/filter/TypeName.h"

#include "smtk/resource/filter/Grammar.h"

namespace smtk
{
namespace graph
{
namespace filter
{
/// The Grammar for smtk::graph builds upon the default Grammar for
/// smtk::resource with the addition of filtering by component type name.
struct SMTKCORE_EXPORT Grammar
  : must<
      sor<
        seq<smtk::graph::filter::TypeName::Grammar, smtk::resource::filter::Grammar>,
        smtk::graph::filter::TypeName::Grammar>,
      eof>
{
};
} // namespace filter
} // namespace graph
} // namespace smtk

#endif
