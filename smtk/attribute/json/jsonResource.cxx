//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "jsonResource.h"
#include "nlohmann/json.hpp"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/json/jsonDefinition.h"
#include "smtk/io/Logger.h"
#include "smtk/resource/json/jsonResource.h"
#include "smtk/view/json/jsonView.h"

#include "smtk/CoreExports.h"

#include <string>

namespace smtk
{
namespace attribute
{
using ItemExpressionDefInfo = std::pair<smtk::attribute::ValueItemDefinitionPtr, std::string>;

using AttRefDefInfo = std::pair<smtk::attribute::RefItemDefinitionPtr, std::string>;

using json = nlohmann::json;

/**\brief Provide a way to serialize Resource. It would stick with attribute
  * V3 format
  */
/// Convert a SelectionManager's currentSelection() to JSON.
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::ResourcePtr& res)
{
  smtk::resource::to_json(j, smtk::static_pointer_cast<smtk::resource::Resource>(res));
  //TODO: write some meta data?
  // Write out the category and analysis information
  if (res->numberOfCategories())
  {
    // When parsing, it might has a default value, so here a value object is added
    j["Categories"]["Value"] = res->categories();
  }
  if (res->numberOfAnalyses())
  {
    j["Analyses"] = res->analyses();
  }

  // Write out the advance levels information
  if (res->numberOfAdvanceLevels())
  {
    json advanceLevelsObj = json::object();
    const std::map<int, std::string>& levels = res->advanceLevels();
    for (auto iter = levels.begin(); iter != levels.end(); iter++)
    {
      int intLevel = iter->first;
      json intLevelObj = json::object();
      // Json mandates key to be string
      intLevelObj["Label"] = iter->second;
      const double* rgba = res->advanceLevelColor(intLevel);
      if (rgba)
      {
        intLevelObj["Color"] = { rgba[0], rgba[1], rgba[2], rgba[3] };
        advanceLevelsObj[std::to_string(intLevel)] = intLevelObj;
      }
    }
    j["AdvanceLevels"] = advanceLevelsObj;
  }

  // In Xml we have control over including definitions, instances,
  // modelInformation and views.

  // Process attribute info
  std::vector<smtk::attribute::DefinitionPtr> definitionPtrs;
  res->definitions(definitionPtrs);
  // a list of defObj
  j["Definitions"] = definitionPtrs;

  // Process views
  // First write toplevel views and then write out the non-toplevel - note that the
  // attribute or view collection do care about this - the assumption
  // is that the designer would probably like all the toplevel views clustered together
  json viewsObj = json::array();
  bool isTop;
  for (auto iter = res->views().begin(); iter != res->views().end(); iter++)
  {
    if (!(iter->second->details().attributeAsBool("TopLevel", isTop) && isTop))
    {
      continue;
    }
    viewsObj.push_back(iter->second);
  }
  for (auto iter = res->views().begin(); iter != res->views().end(); iter++)
  {
    if ((iter->second->details().attributeAsBool("TopLevel", isTop) && isTop))
    {
      continue;
    }
    viewsObj.push_back(iter->second);
  }
  j["Views"] = viewsObj;

  // Process model info
}

SMTKCORE_EXPORT void from_json(const json& j, smtk::attribute::ResourcePtr& res)
{
  std::vector<smtk::attribute::ItemExpressionDefInfo> expressDefInfo;
  std::vector<smtk::attribute::AttRefDefInfo> refDefInfo;
  smtk::io::Logger logger;
  //TODO: v2Parser has a notion of rootName
  if (!res.get() || j.is_null())
  {
    // Create a valid resourcePtr so we can assign it to someone else
    res = smtk::attribute::Resource::create();
  }

  auto temp = std::static_pointer_cast<smtk::resource::Resource>(res);
  smtk::resource::from_json(j, temp);

  // Process Analysis Info
  // nlohmman's get function does not support nested map, so iterator is used
  try
  {
    json analyses = j.at("Analyses");
    for (json::iterator iterAna = analyses.begin(); iterAna != analyses.end(); iterAna++)
    {
      res->defineAnalysis(iterAna.key(), iterAna.value());
    }
  }
  catch (std::exception& /*e*/)
  {
  }

  //Process AdvanceLevel info
  try
  {
    json advanceLevels = j.at("AdvanceLevels");
    for (auto iterAdv = advanceLevels.begin(); iterAdv != advanceLevels.end(); iterAdv++)
    {
      int level = std::stoi(iterAdv.key());
      json levelObj = iterAdv.value();
      std::vector<double> rgba = levelObj.at("Color");
      std::string label = levelObj.at("Label");
      double color[4];
      for (int i = 0; i < 4; i++)
      {
        color[i] = rgba[i];
      }
      res->addAdvanceLevel(level, label);
      res->setAdvanceLevelColor(level, color);
    }
  }
  catch (std::exception& /*e*/)
  {
  }

  // Process attribute info
  json defininitions;
  try
  {
    defininitions = j.at("Definitions");
  }
  catch (std::exception& /*e*/)
  {
  }
  if (!defininitions.is_null())
  {
    for (auto iterDef = defininitions.begin(); iterDef != defininitions.end(); iterDef++)
    {
      try
      {
        smtk::attribute::DefinitionPtr def, baseDef;
        json currentDef = *iterDef;
        // Get type and baseDef info
        std::string type = currentDef.at("Type");
        if (type.empty())
        {
          smtkErrorMacro(logger, "Definition missing Type XML Attribute");
          continue;
        }
        std::string baseType = currentDef.at("BaseType").is_null() ? "" : currentDef.at("BaseType");
        if (!baseType.empty())
        {
          baseDef = res->findDefinition(baseType);
          if (!baseDef)
          {
            smtkErrorMacro(logger, "Could not find Base Definition: "
                << baseType << " needed to create Definition: " << type);
            continue;
          }
          def = res->createDefinition(type, baseDef);
        }
        else
        {
          def = res->createDefinition(type);
        }
        if (!def)
        {
          smtkWarningMacro(logger, "Definition: " << type << " already exists in the Resource");
          continue;
        }
        // process the definition
        // Since definition is not default constructible, we have to call the
        // function directly
        // expressionDefInfo, refDefInfo and attributes are handled in jsonDefinition
        smtk::attribute::from_json(currentDef, def, expressDefInfo, refDefInfo);
      }
      catch (std::exception& /*e*/)
      {
      }
    }
  }

  // Proces view info
  try
  {
    json views = j.at("Views");
    for (auto iterView = views.begin(); iterView != views.end(); iterView++)
    {
      smtk::view::ViewPtr view = *iterView;
      res->addView(view);
    }
  }
  catch (std::exception& /*e*/)
  {
  }
  // Process model info

  // Update category infomration
  res->updateCategories();
}
}
}
