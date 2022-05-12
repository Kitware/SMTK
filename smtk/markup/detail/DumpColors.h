//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_detail_DumpColors_h
#define smtk_markup_detail_DumpColors_h

#include "smtk/markup/Exports.h"
#include "smtk/string/Token.h"

#include <array>
#include <map>

namespace smtk
{
namespace markup
{

SMTK_ALWAYS_EXPORT inline const std::map<smtk::string::Token, std::array<double, 4>>&
DumpArcColors()
{
  using namespace smtk::string::literals; // for ""_token
  static std::map<smtk::string::Token, std::array<double, 4>> arcColors{
    { { "smtk::markup::arcs::BoundariesToShapes"_token, { 0.00, 0.00, 0.00, 1.00 } },
      { "smtk::markup::arcs::FieldsToShapes"_token, { 1.00, 0.00, 0.00, 1.00 } },
      { "smtk::markup::arcs::GroupsToMembers"_token, { 0.00, 1.00, 0.00, 1.00 } },
      { "smtk::markup::arcs::LabelsToSubjects"_token, { 0.00, 0.00, 1.00, 1.00 } },
      { "smtk::markup::arcs::URLsToData"_token, { 1.00, 0.00, 1.00, 1.00 } } }
  };
  return arcColors;
}

} // namespace markup
} // namespace smtk

#endif // smtk_markup_detail_DumpColors_h
