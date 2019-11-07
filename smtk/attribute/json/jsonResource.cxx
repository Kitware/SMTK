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
    std::vector<std::string> rInfo;
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
      if (analysis->isRequired())
      {
        rInfo.push_back(analysis->name());
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
    if (rInfo.size())
    {
      j["AnalysesRequiredInfo"] = rInfo;
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
  //TODO: v2Parser has a notion of rootName
  if (!res.get() || j.is_null())
  {
    // Create a valid resourcePtr so we can assign it to someone else
    res = smtk::attribute::Resource::create();
  }

  auto resource = std::static_pointer_cast<smtk::resource::Resource>(res);
  smtk::resource::from_json(j, resource);

  // Process Analysis Info
  auto result = j.find("Analyses");
  if (result != j.end())
  {
    smtk::attribute::Analyses& analyses = res->analyses();
    std::vector<std::string> eInfo;
    // we need this for backward compatibility
    auto aInfo = result->get<std::map<std::string, std::set<std::string> > >();

    // Get the order of analysis creation if it exists
    auto analysesOrder = j.find("AnalysesOrder");
    if (analysesOrder != j.end())
    {
      auto aNames = analysesOrder->get<std::vector<std::string> >();
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
          smtkErrorMacro(
            smtk::io::Logger::instance(), "Analysis: " << name << " missing local categories!");
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
    auto analysesParentInfo = j.find("AnalysesParentInfo");
    if (analysesParentInfo != j.end())
    {
      for (auto const& val : analysesParentInfo->items())
      {
        if (!analyses.setAnalysisParent(val.key(), val.value()))
        {
          smtkErrorMacro(smtk::io::Logger::instance(),
            "Analysis: " << val.key() << " could not set parent to: " << val.value() << "!");
        }
      }
    }
    // What about Exclusive Info?
    auto analysesExclusiveInfo = j.find("AnalysesExclusiveInfo");
    if (analysesExclusiveInfo != j.end())
    {
      auto eNames = analysesExclusiveInfo->get<std::vector<std::string> >();
      for (auto const& name : eNames)
      {
        auto a = analyses.find(name);
        if (a != nullptr)
        {
          a->setExclusive(true);
        }
        else
        {
          smtkErrorMacro(smtk::io::Logger::instance(),
            "Could not find Analysis: " << name << " to set its exclusive property!");
        }
      }
    }
    // What about Required Info?
    auto analysesRequiredInfo = j.find("AnalysesRequiredInfo");
    if (analysesRequiredInfo != j.end())
    {
      auto rNames = analysesRequiredInfo->get<std::vector<std::string> >();
      for (auto const& name : rNames)
      {
        auto a = analyses.find(name);
        if (a != nullptr)
        {
          a->setRequired(true);
        }
        else
        {
          smtkErrorMacro(smtk::io::Logger::instance(),
            "Could not find Analysis: " << name << " to set its required property!");
        }
      }
    }
    auto analysesLabelInfo = j.find("AnalysesLabelInfo");
    // Do we have label infomation to deal with?
    if (analysesLabelInfo != j.end())
    {
      for (auto const& val : analysesLabelInfo->items())
      {
        auto a = analyses.find(val.key());
        if (a != nullptr)
        {
          a->setLabel(val.value());
        }
        else
        {
          smtkErrorMacro(smtk::io::Logger::instance(),
            "Could not find Analysis: " << val.key() << " to set its label property!");
        }
      }
    }
    auto analysesTopLevelExclusive = j.find("AnalysesTopLevelExclusive");
    if (analysesTopLevelExclusive != j.end())
    {
      analyses.setTopLevelExclusive(analysesTopLevelExclusive->get<bool>());
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
  auto advanceLevels = j.find("AdvanceLevels");
  if (advanceLevels != j.end())
  {
    //    for (auto& levelInfo : *advanceLevels)
    for (auto levelInfo = advanceLevels->begin(); levelInfo != advanceLevels->end(); levelInfo++)
    {
      int level = std::stoi(levelInfo.key());
      json levelObj = levelInfo.value();
      auto color = levelObj.find("Color");
      if ((color != levelObj.end()) && (color->size() == 4))
      {
        double rgba[4];
        rgba[0] = (*color)[0];
        rgba[1] = (*color)[1];
        rgba[2] = (*color)[2];
        rgba[3] = (*color)[3];
        res->setAdvanceLevelColor(level, rgba);
      }
      auto label = levelObj.find("Label");
      if (label != levelObj.end())
      {
        res->addAdvanceLevel(level, *label);
      }
    }
  }

  // Process Definiiton info
  std::set<const smtk::attribute::ItemDefinition*> convertedAttDefs;
  auto definitions = j.find("Definitions");
  if (definitions != j.end())
  {
    smtk::attribute::DefinitionPtr def, baseDef;
    for (auto& currentDef : *definitions)
    {
      auto type = currentDef.find("Type");
      if (type == currentDef.end())
      {
        smtkErrorMacro(smtk::io::Logger::instance(), "Definition missing Type Key");
        continue;
      }
      auto baseType = currentDef.find("BaseType");
      if ((baseType == currentDef.end()) || (*baseType == ""))
      {
        baseDef = nullptr;
      }
      else
      {
        baseDef = res->findDefinition(*baseType);
        if (!baseDef)
        {
          smtkErrorMacro(smtk::io::Logger::instance(), "Could not find Base Definition: "
              << *baseType << " needed to create Definition: " << *type);
          continue;
        }
      }
      def = res->createDefinition(*type, baseDef);
      if (!def)
      {
        smtkWarningMacro(smtk::io::Logger::instance(),
          "Definition: " << *type << " already exists in the Resource");
        continue;
      }
      // process the definition
      // Since definition is not default constructible, we have to call the
      // function directly
      smtk::attribute::from_json(currentDef, def, convertedAttDefs);
    }
  }

  // Check for Exclusions
  auto exclusions = j.find("Exclusions");
  if (exclusions != j.end())
  {
    for (auto& exclusion : *exclusions)
    {
      // First lets convert the strings to definitions
      std::vector<smtk::attribute::DefinitionPtr> defs;
      smtk::attribute::DefinitionPtr def;
      for (auto& defName : exclusion)
      {
        def = res->findDefinition(defName);
        if (def)
        {
          defs.push_back(def);
        }
        else
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(), "Cannot find exclusion definiion: " << defName);
        }
      }
      // Now create the exclusions
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
  auto prerequisites = j.find("Prerequisites");
  if (prerequisites != j.end())
  {
    for (auto& pInfo : *prerequisites)
    {
      auto type = pInfo.find("Type");
      if (type == pInfo.end())
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(), "Cannot find Type Key - Skipping Prerequisite");
        continue;
      }
      // Lets find the target definition
      auto tdef = res->findDefinition(*type);
      if (!tdef)
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(), "Cannot find Prerequisite target definition: " << *type);
        continue;
      }

      auto prerequisite = pInfo.find("Prerequisite");
      if (prerequisite == pInfo.end())
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(), "Cannot find Prerequisite Key - Skipping Prerequisite");
        continue;
      }

      // OK now lets add the prerequisites to the definition
      smtk::attribute::DefinitionPtr def;
      for (auto& defName : *prerequisite)
      {
        def = res->findDefinition(defName);
        if (def)
        {
          tdef->addPrerequisite(def);
        }
        else
        {
          smtkErrorMacro(smtk::io::Logger::instance(),
            "Cannot find prerequisite definiion: " << defName << " for Definition: " << *type);
        }
      }
    }
  }

  // Process attributes info
  std::vector<ItemExpressionInfo> itemExpressionInfo;
  std::vector<AttRefInfo> attRefInfo;
  smtk::attribute::AttributePtr att;
  auto attributes = j.find("Attributes");
  if (attributes != j.end())
  {
    for (auto& jAtt : *attributes)
    {
      auto name = jAtt.find("Name");
      if (name == jAtt.end())
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(), "Invalid Attribute! - Missing json Attribute Name");
        continue;
      }
      // Lets get the defintion for the attribute
      auto type = jAtt.find("Type");
      if (type == jAtt.end())
      {
        smtkErrorMacro(smtk::io::Logger::instance(),
          "Invalid Attribute! - Missing Type for attribute:" << *name);
        continue;
      }
      smtk::attribute::DefinitionPtr def = res->findDefinition(*type);
      if (def == nullptr)
      {
        smtkErrorMacro(smtk::io::Logger::instance(),
          "Invalid Attribute! - Cannot find Definition of Type:" << *type
                                                                 << " for attribute:" << *name);
        continue;
      }
      // Is the definition abstract?
      if (def->isAbstract())
      {
        smtkErrorMacro(smtk::io::Logger::instance(), "Attribute: "
            << *name << " of Type: " << *type << "  - is based on an abstract definition");
        continue;
      }

      auto id = jAtt.find("ID");
      if (id == jAtt.end())
      {
        smtkErrorMacro(smtk::io::Logger::instance(),
          "Invalid Attribute! - Missing ID for attribute:" << *name << " of type:" << *type);
        continue;
      }
      smtk::common::UUID uuid(id->get<std::string>());

      // Ok we can now create the attribute
      att = res->createAttribute(*name, def, uuid);

      if (att == nullptr)
      {
        smtkErrorMacro(smtk::io::Logger::instance(), "Attribute: "
            << *name << " of Type: " << *type << "  - could not be created - is the name in use?");
        continue;
      }
      smtk::attribute::from_json(jAtt, att, itemExpressionInfo, attRefInfo, convertedAttDefs);
    }
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
      smtkErrorMacro(smtk::io::Logger::instance(), "Expression Attribute: "
          << itemExpressionInfo[i].expName
          << " is missing and required by Item : " << itemExpressionInfo[i].item->name());
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
      smtkErrorMacro(smtk::io::Logger::instance(), "Referenced Attribute: "
          << attRefInfo[i].attName
          << " is missing and required by Item: " << attRefInfo[i].item->name());
    }
  }

  // Proces view info
  auto views = j.find("Views");
  if (views != j.end())
  {
    for (auto& jView : *views)
    {
      smtk::view::ViewPtr view = jView;
      res->addView(view);
    }
  }

  // Update category infomration
  res->updateCategories();
}
}
}
