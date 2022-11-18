//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/URL.h"

#include "smtk/markup/Traits.h"

#include "smtk/common/Paths.h"

namespace smtk
{
namespace markup
{

URL::~URL() = default;

bool URL::setLocation(const smtk::string::Token& location)
{
  auto tokens = this->properties().get<smtk::string::Token>();
  if (tokens.contains("location") && tokens.at("location") == location)
  {
    return false;
  }
  tokens["location"] = location;
  return true;
}

smtk::string::Token URL::location() const
{
  if (!this->properties().contains<smtk::string::Token>("location"))
  {
    return smtk::string::Token();
  }
  return this->properties().at<smtk::string::Token>("location");
}

bool URL::setType(const smtk::string::Token& mimeType)
{
  auto tokens = this->properties().get<smtk::string::Token>();
  if (tokens.contains("mime_type") && tokens.at("mime_type") == mimeType)
  {
    return false;
  }
  tokens["mime_type"] = mimeType;
  return true;
}

smtk::string::Token URL::type() const
{
  if (!this->properties().contains<smtk::string::Token>("mime_type"))
  {
    return smtk::string::Token();
  }
  return this->properties().at<smtk::string::Token>("mime_type");
}

std::string URL::extensionForType() const
{
  using namespace smtk::string::literals;

  std::string ext = smtk::common::Paths::extension(this->location().data());
  if (!ext.empty())
  {
    return ext;
  }

  auto mimeType = this->type();
  switch (mimeType.id())
  {
    case "vtk/unstructured-grid"_hash: // fall through
    case "vtk/polydata"_hash:          // fall through
    case "vtk"_hash:
      ext = ".vtk";
      break;

    case "vtk/image"_hash: // fall through
    case "vti"_hash:
      ext = ".vti";
      break;

    default: // do nothing
      break;
  }
  return ext;
}

ArcEndpointInterface<arcs::URLsToData, ConstArc, OutgoingArc> URL::data() const
{
  return this->outgoing<arcs::URLsToData>();
}

ArcEndpointInterface<arcs::URLsToData, NonConstArc, OutgoingArc> URL::data()
{
  return this->outgoing<arcs::URLsToData>();
}

} // namespace markup
} // namespace smtk
