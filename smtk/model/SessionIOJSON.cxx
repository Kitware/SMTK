//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/SessionIOJSON.h"

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/common/CompilerInformation.h"
#include "smtk/common/UUID.h"
#include "smtk/model/Resource.h"

#include "smtk/model/json/jsonResource.h"

#include <fstream>

using json = nlohmann::json;

namespace smtk
{
namespace model
{

json SessionIOJSON::saveJSON(const smtk::model::ResourcePtr& rsrc)
{
  json result = rsrc;
  return rsrc;
}

bool SessionIOJSON::saveModelRecords(const json& j, const std::string& url)
{
  if (url.empty())
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "A filename must be specified.");
    return false;
  }
  std::ofstream file(url);
  if (!file.good())
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Unable to open \"" << url << "\" for writing.");
    return false;
  }
  file << j.dump(2);
  file.close();
  return true;
}

json SessionIOJSON::loadJSON(const std::string& filename)
{
  json result;
  std::ifstream file(filename);
  if (!file.good())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Could not open \"" << filename << "\" for reading.");
    return result;
  }
  try
  {
    result = json::parse(file);
  }
  catch (...)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "File \"" << filename << "\" is not in JSON format.");
  }
  return result;
}

bool SessionIOJSON::loadModelRecords(const json& j, smtk::model::ResourcePtr rsrc)
{
  if (j.is_null())
  {
    return false;
  }

  smtk::model::from_json(j, rsrc);

  rsrc->setClean();
  return true;
}

} // namespace model
} // namespace smtk
