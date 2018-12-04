//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_jsonResourceDescriptor_h
#define smtk_project_jsonResourceDescriptor_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"

#include "smtk/common/UUID.h"
#include "smtk/project/ResourceDescriptor.h"

#include "nlohmann/json.hpp"

#include <string>
#include <vector>

using json = nlohmann::json;

namespace smtk
{
namespace project
{
static void to_json(json& j, const ResourceDescriptor& rd)
{
  j = { { "filename", rd.m_filename }, { "identifier", rd.m_identifier },
    { "importLocation", rd.m_importLocation }, { "typeName", rd.m_typeName },
    { "uuid", rd.m_uuid.toString() } };
} // to_json()

static void from_json(const json& j, ResourceDescriptor& rd)
{
  try
  {
    rd.m_filename = j.at("filename");
    rd.m_identifier = j.at("identifier");
    rd.m_importLocation = j.at("importLocation");
    rd.m_typeName = j.at("typeName");

    std::string uuidString = j.at("uuid");
    rd.m_uuid = smtk::common::UUID(uuidString);
  }
  catch (std::exception& ex)
  {
    std::cerr << "ERROR: " << ex.what();
  }
} // from_json()

} // namespace project
} // namespace smtk

#endif // smtk_project_jsonResourceDescriptor_h
