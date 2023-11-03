//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_project_filter_Grammar_h
#define smtk_project_filter_Grammar_h

#include "smtk/CoreExports.h"

#include "smtk/project/filter/TypeName.h"

#include "smtk/resource/filter/Grammar.h"

namespace smtk
{
namespace project
{
/// Query-string parsing functionality for projects.
namespace filter
{
/// The Grammar for smtk::project builds upon the default Grammar for
/// smtk::resource with the addition of filtering by component type name.
struct SMTKCORE_EXPORT Grammar
  : must<
      sor<
        seq<smtk::project::filter::TypeName::Grammar, smtk::resource::filter::Grammar>,
        smtk::project::filter::TypeName::Grammar>,
      tao::pegtl::eof>
{
};
} // namespace filter
} // namespace project
} // namespace smtk

#endif
