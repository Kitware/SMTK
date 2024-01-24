//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/Logger.h"

#include "smtk/markup/operators/CreateArc.h"

#include "smtk/common/StringUtil.h"

#include <sstream>

// On Windows MSVC 2015+, something is included that defines
// a macro named ERROR to be 0. This causes smtkErrorMacro()
// to expand into garbage (because smtk::io::Logger::ERROR
// gets expanded to smtk::io::Logger::0).
#ifdef ERROR
#undef ERROR
#endif

using namespace smtk::string::literals;

namespace smtk
{
namespace markup
{

CreateArc::CreateArc() = default;

const char* CreateArc::xmlDescription() const
{
  static std::string description;
  if (description.empty())
  {
    description = this->Superclass::xmlDescription();
    smtk::common::StringUtil::replaceAll(
      description, "smtk::graph::Component", "smtk::markup::Component");
    smtk::common::StringUtil::replaceAll(
      description, "smtk::graph::ResourceBase", "smtk::markup::Resource");
  }

  return description.c_str();
}

} // namespace markup
} // namespace smtk
