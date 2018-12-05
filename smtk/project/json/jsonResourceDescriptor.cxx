//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/project/json/jsonResourceDescriptor.h"

#include "smtk/common/UUID.h"

using json = nlohmann::json;

namespace smtk
{
namespace project
{
void to_json(json& j, const ResourceDescriptor& rd)
{
  j = { { "filename", rd.m_filename }, { "identifier", rd.m_identifier },
    { "importLocation", rd.m_importLocation }, { "typeName", rd.m_typeName },
    { "uuid", rd.m_uuid.toString() } };
} // to_json()

void from_json(const json& j, ResourceDescriptor& rd)
{
  rd.m_filename = j.at("filename");
  rd.m_identifier = j.at("identifier");
  rd.m_importLocation = j.at("importLocation");
  rd.m_typeName = j.at("typeName");

  std::string uuidString = j.at("uuid");
  rd.m_uuid = smtk::common::UUID(uuidString);
} // from_json()

} // namespace project
} // namespace smtk
