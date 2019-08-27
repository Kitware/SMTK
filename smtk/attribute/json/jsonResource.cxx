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
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/json/jsonAttribute.h"
#include "smtk/attribute/json/jsonDefinition.h"
#include "smtk/io/Logger.h"
#include "smtk/resource/json/jsonResource.h"
#include "smtk/view/json/jsonView.h"

#include "smtk/CoreExports.h"

#include <queue>
#include <string>

namespace smtk
{
namespace attribute
{
using json = nlohmann::json;

/// \brief Provide a way to serialize an attribute::Resource. The current version is 4.0 but
/// but can also read in a resource in version 3.0 format.
SMTKCORE_EXPORT void to_json(json& j, const smtk::attribute::ResourcePtr& res)
{
  smtk::resource::to_json(j, smtk::static_pointer_cast<smtk::resource::Resource>(res));
  // Set the version to 4.0
  j["version"] = "4.0";
  // Write out the category and analysis information
  if (res->numberOfCategories())
  {
    // When parsing, it might has a default value, so here a value object is added
    j["Categories"]["Value"] = res->categories();
  }
  smtk::attribute::Analyses& analyses = res->analyses();
  if (analyses.size())
  {
    std::map<std::string, std::string> pInfo, lInfo;
    std::vector<std::string> aNames;
    std::vector<std::string> eInfo;
    // we need this for backward compatibility
    std::map<std::string, std::set<std::string> > aInfo;
    for (auto analysis : analyses.analyses())
    {
      aNames.push_back(analysis->name());
      aInfo[analysis->name()] = analysis->localCategories();
      if (analysis->parent() != nullptr)
      {
        pInfo[analysis->name()] = analysis->parent()->name();
      }
      if (analysis->isExclusive())
      {
        eInfo.push_back(analysis->name());
      }
      if (analysis->hasLabel())
      {
        lInfo[analysis->name()] = analysis->label();
      }
    }
    j["Analyses"] = aInfo;
    j["AnalysesOrder"] = aNames;
    if (pInfo.size())
    {
      j["AnalysesParentInfo"] = pInfo;
    }
    if (lInfo.size())
    {
      j["AnalysesLabelInfo"] = lInfo;
    }
    if (eInfo.size())
    {
      j["AnalysesExclusiveInfo"] = eInfo;
    }
    if (analyses.areTopLevelExclusive())
    {
      j["AnalysesTopLevelExclusive"] = true;
    }
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

  // Do we have unique roles to be saved?
  const std::set<smtk::resource::Links::RoleType>& roles = res->uniqueRoles();
  if (!roles.empty())
  {
    j["UniqueRoles"] = roles;
  }
  // In Xml we have control over including definitions, instances,
  // modelInformation and views.

  // Process definifions from base to derived so that
  // when deserializing defs the base def would be available when processing
  // derived defs. Attributes are also processed in the loop for simplicity.
  std::vector<smtk::attribute::DefinitionPtr> baseDefPtrs, derivedDefPtrs;
  res->findBaseDefinitions(baseDefPtrs);
  json defsObj = json::array();
  json attsObj = json::array();
  json excsObj = json::array();
  json presObj = json::array();

  std::queue<smtk::attribute::DefinitionPtr, std::deque<smtk::attribute::DefinitionPtr> > defsQueue(
    std::deque<smtk::attribute::DefinitionPtr>(baseDefPtrs.begin(), baseDefPtrs.end()));
  while (!defsQueue.empty())
  {
    smtk::attribute::DefinitionPtr currentDef = defsQueue.front();
    defsObj.push_back(currentDef);

    std::vector<smtk::attribute::AttributePtr> atts;
    res->findDefinitionAttributes(currentDef->type(), atts);
    for (const auto& att : atts)
    {
      attsObj.push_back(att);
    }

    defsQueue.pop();

    res->derivedDefinitions(currentDef, derivedDefPtrs);
    for (const auto& derivedDefPtr : derivedDefPtrs)
    {
      defsQueue.push(derivedDefPtr);
    }
  }
  j["Definitions"] = defsObj;

  // Process Exceptions and Prerequistics
  std::vector<smtk::attribute::DefinitionPtr> defs;
  res->definitions(defs, true);
  for (auto def : defs)
  {
    auto defType = def->type();
    // Lets process the constraints of def

    auto excludedTypes = def->excludedTypeNames();
    if (excludedTypes.size())
    {
      json types = json::array();
      for (auto etype : excludedTypes)
      {
        if (etype > defType)
        {
          types.push_back(defType);
          types.push_back(etype);
        }
      }
      if (types.size())
      {
        excsObj.push_back(types);
      }
    }
    // Now the prerequistics
    auto prerequisitesTypes = def->prerequisiteTypeNames();
    if (prerequisitesTypes.size())
    {
      json pobj = json::object();
      pobj["Type"] = defType;
      json types(prerequisitesTypes);
      pobj["Prerequisite"] = types;
      presObj.push_back(pobj);
    }
  }

  if (excsObj.size())
  {
    j["Exclusions"] = excsObj;
  }

  if (presObj.size())
  {
    j["Prerequisites"] = presObj;
  }

  j["Attributes"] = attsObj;

  // Process views
  // First write toplevel views and then write out the non-toplevel - note that the
  // attribute resource or views do not care about this - the assumption
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
  if (j.find("Analyses") != j.end())
  {
    smtk::attribute::Analyses& analyses = res->analyses();
    json jAnalyses = j.at("Analyses");
    std::vector<std::string> eInfo;
    // we need this for backward compatibility
    auto aInfo = j.at("Analyses").get<std::map<std::string, std::set<std::string> > >();

    // Get the order of analysis creation if it exists
    if (j.find("AnalysesOrder") != j.end())
    {
      auto aNames = j.at("AnalysesOrder").get<std::vector<std::string> >();
      for (auto const& name : aNames)
      {
        // Lets find its local categories
        auto it = aInfo.find(name);
        if (it != aInfo.end())
        {
          auto analysis = analyses.create(name);
          analysis->setLocalCategories(it->second);
        }
        else
        {
          smtkErrorMacro(logger, "Analysis: " << name << " missing local categories!");
        }
      }
    }
    else
    {
      // There is no specified order so use the dictionary itself
      for (auto const& info : aInfo)
      {
        auto analysis = analyses.create(info.first);
        analysis->setLocalCategories(info.second);
      }
    }
    // Do we have parent infomation to deal with?
    if (j.find("AnalysesParentInfo") != j.end())
    {
      json parentInfo = j.at("AnalysesParentInfo");
      for (auto const& val : parentInfo.items())
      {
        if (!analyses.setAnalysisParent(val.key(), val.value()))
        {
          smtkErrorMacro(
            logger, "Analsis: " << val.key() << " could not set parent to: " << val.value() << "!");
        }
      }
    }
    // What about Exclusive Info?
    if (j.find("AnalysesExclusiveInfo") != j.end())
    {
      auto eNames = j.at("AnalysesExclusiveInfo").get<std::vector<std::string> >();
      for (auto const& name : eNames)
      {
        auto a = analyses.find(name);
        if (a != nullptr)
        {
          a->setExclusive(true);
        }
        else
        {
          smtkErrorMacro(
            logger, "Could not find Analysis: " << name << " to set its exclusive property!");
        }
      }
    }
    // Do we have label infomation to deal with?
    if (j.find("AnalysesLabelInfo") != j.end())
    {
      json labelInfo = j.at("AnalysesLabelInfo");
      for (auto const& val : labelInfo.items())
      {
        auto a = analyses.find(val.key());
        if (a != nullptr)
        {
          a->setLabel(val.value());
        }
        else
        {
          smtkErrorMacro(
            logger, "Could not find Analysis: " << val.key() << " to set its label property!");
        }
      }
    }
    if (j.find("AnalysesTopLevelExclusive") != j.end())
    {
      analyses.setTopLevelExclusive(j.at("AnalysesTopLevelExclusive").get<bool>());
    }
  }
  // Do we have unique roles?
  auto uniqueRoles = j.find("UniqueRoles");
  if (uniqueRoles != j.end())
  {
    for (auto role : *uniqueRoles)
    {
      res->addUniqueRole(role);
    }
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
  std::set<const smtk::attribute::ItemDefinition*> convertedAttDefs;
  try
  {
    json definitions = j.at("Definitions");
    for (auto iterDef = definitions.begin(); iterDef != definitions.end(); iterDef++)
    {
      try
      {
        smtk::attribute::DefinitionPtr def, baseDef;
        json currentDef = *iterDef;
        // Get type and baseDef info
        std::string type = currentDef.at("Type");
        if (type.empty())
        {
          smtkErrorMacro(logger, "Definition missing Type Key");
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
        smtk::attribute::from_json(currentDef, def, convertedAttDefs);
      }
      catch (std::exception& /*e*/)
      {
        std::cerr << "Failed to find type of a definition for resource " << res->name()
                  << std::endl;
      }
    }
  }
  catch (std::exception& /*e*/)
  {
    std::cerr << "Failed to find definitions for resource " << res->name() << std::endl;
  }
  // Check for Exclusions
  if (j.find("Exclusions") != j.end())
  {
    auto excsObj = j.at("Exclusions");
    for (auto excsInter = excsObj.begin(); excsInter != excsObj.end(); excsInter++)
    {
      auto excObj = *excsInter; // Get the exclusion list
      // First lets convert the strings to definitions
      std::vector<smtk::attribute::DefinitionPtr> defs;
      smtk::attribute::DefinitionPtr def;
      for (auto strIter = excObj.begin(); strIter != excObj.end(); strIter++)
      {
        def = res->findDefinition(strIter->get<std::string>());
        if (def)
        {
          defs.push_back(def);
        }
        else
        {
          std::cerr << "Cannot find exclusion definiion: " << strIter->get<std::string>()
                    << std::endl;
        }
      }
      auto defsSize = defs.size();
      for (size_t i = 0; i < defsSize; i++)
      {
        for (size_t k = i + 1; k < defsSize; k++)
        {
          defs[i]->addExclusion(defs[k]);
        }
      }
    }
  }

  // Check for Prerequisites
  if (j.find("Prerequisites") != j.end())
  {
    auto presObj = j.at("Prerequisites");
    smtk::attribute::DefinitionPtr def;
    for (auto presInter = presObj.begin(); presInter != presObj.end(); presInter++)
    {
      auto preObj = *presInter;
      if (preObj.find("Type") == preObj.end())
      {
        std::cerr << "Cannot find Type Key - Skipping Prerequisite\n";
        continue;
      }
      if (preObj.find("Prerequisite") == preObj.end())
      {
        std::cerr << "Cannot find Prerequisite Key - Skipping Prerequisite\n";
        continue;
      }
      std::string tname = preObj.at("Type").get<std::string>();
      // Lets find the target definition
      auto tdef = res->findDefinition(tname);
      if (!tdef)
      {
        std::cerr << "Cannot find target definition: " << tname << std::endl;
        continue;
      }
      auto preDefs = preObj.at("Prerequisite");
      for (auto strIter = preDefs.begin(); strIter != preDefs.end(); strIter++)
      {
        def = res->findDefinition(strIter->get<std::string>());
        if (def)
        {
          tdef->addPrerequisite(def);
        }
        else
        {
          std::cerr << "Cannot find prerequisite definiion: " << strIter->get<std::string>()
                    << " for Definition: " << tname << std::endl;
        }
      }
    }
  }

  // Process attributes info
  std::vector<ItemExpressionInfo> itemExpressionInfo;
  std::vector<AttRefInfo> attRefInfo;
  smtk::attribute::AttributePtr att;
  try
  {
    json attributes = j.at("Attributes");
    smtk::attribute::DefinitionPtr def;
    for (auto iter = attributes.begin(); iter != attributes.end(); iter++)
    { // Get/Create the attribute first
      std::string name, type;
      try
      {
        name = iter->at("Name");
      }
      catch (std::exception& /*e*/)
      {
      }

      if (name.empty())
      {
        std::cerr << "Invalid Attribute! - Missing json Attribute Name" << std::endl;
        continue;
      }
      try
      {
        type = iter->at("Type");
      }
      catch (std::exception& /*e*/)
      {
      }
      if (type.empty())
      {
        std::cerr << "Invalid Attribute! - Missing json Attribute type" << std::endl;
        continue;
      }
      smtk::common::UUID uuid = smtk::common::UUID::null();
      try
      {
        std::string uuidString = iter->at("ID");
        uuid = smtk::common::UUID(uuidString);
      }
      catch (std::exception& /*e*/)
      {
      };
      def = res->findDefinition(type);
      if (!def)
      {
        std::cerr << "Attribute: " << name << " of Type: " << type
                  << "  - can not find attribute definition" << std::endl;
        continue;
      }

      // Is the definition abstract?
      if (def->isAbstract())
      {
        std::cerr << "Attribute: " << name << " of Type: " << type
                  << "  - is based on an abstract definition" << std::endl;
        continue;
      }

      if (uuid.isNull())
      {
        std::cerr << "uuid is null for Attribute " << name << std::endl;
        continue;
      }
      else
      {
        att = res->createAttribute(name, def, uuid);
      }

      if (!att)
      {
        std::cerr << "Attribute: " << name << " of Type: " << type
                  << "  - could not be created - is the name in use" << std::endl;
        continue;
      }
      smtk::attribute::from_json(*iter, att, itemExpressionInfo, attRefInfo, convertedAttDefs);
    }
  }
  catch (std::exception& /*e*/)
  {
    std::cerr << "Failed to find attributes for resource " << res->name() << std::endl;
  }
  // At this point we have all the attributes read in so lets
  // fix up all of the attribute references
  for (size_t i = 0; i < itemExpressionInfo.size(); i++)
  {
    att = res->findAttribute(itemExpressionInfo[i].expName);
    if (att)
    {
      itemExpressionInfo[i].item->setExpression(itemExpressionInfo[i].pos, att);
    }
    else
    {
      std::cerr << "Expression Attribute: " << itemExpressionInfo[i].expName
                << " is missing and required by Item : " << itemExpressionInfo[i].item->name()
                << std::endl;
    }
  }
  for (size_t i = 0; i < attRefInfo.size(); i++)
  {
    att = res->findAttribute(attRefInfo[i].attName);
    if (att)
    {
      attRefInfo[i].item->setValue(attRefInfo[i].pos, att);
    }
    else
    {
      std::cerr << "Referenced Attribute: " << attRefInfo[i].attName
                << " is missing and required by Item: " << attRefInfo[i].item->name() << std::endl;
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
